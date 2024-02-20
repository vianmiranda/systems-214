#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h" //reference the related header file

// Makes sure size is rounded to nearest multiple of 8
#define REALIGN8(x) (((x)+7) & (~7))

// Define metadata size 
// 1. size_t for size 
// 2. int for allocation status, either 1 for taken or 0 for free
#define HEADER_SIZE (REALIGN8(sizeof(chunkheader)))

// Amount of memory allocation for global array
#define MEMLENGTH  512
static double memory[MEMLENGTH];

// 0 = free, 1 = taken
enum allocation_status {
    FREE,
    TAKEN
};

// 1. Header (metadata) - informs runtime system about chunk size and whether the chunk is allocated
// 2. Payload (data) - the actual object/data, a contiguous sequence of bytes
typedef struct {
    size_t size;
    int is_allocated;
} chunkheader;

typedef char chunkpayload;

typedef struct {
    chunkheader header;
    chunkpayload payload;
} chunk;


// Checks first int for the size of the header
size_t getChunkSize(chunkheader* head) {
    return head->size;
}

// Set the size of a chunk
void setChunkSize(chunkheader* head, size_t size) {
    head->size = size;
}

// Checks second int for the allocation status
// Returns 1 for taken, 0 for free
int getAllocationStatus(chunkheader* head) {
    return head->is_allocated == TAKEN;
}

// Marks as 0 for free
void setFree(chunkheader* head) {
    head->is_allocated = FREE;
}

// Marks as 1 for taken
void setAllocated(chunkheader* head) {
    head->is_allocated = TAKEN;
}

chunkpayload* getPayload(chunkheader* head) {
    return (chunkpayload*) (((char*) head) + HEADER_SIZE);
}

// Returns the pointer to the next chunk
chunk* getNextChunk(chunkheader* head) {
    return (chunk*) (((char*) head) + getChunkSize(head));
}

// Returns 1 if memory is cleared and properly coalesced, 0 otherwise
int getMemoryStatus() {
    chunkheader* start = (chunkheader*) memory;
    size_t chunkSize = getChunkSize(start);
    int isFree = getAllocationStatus(start) == FREE;

    if (isFree && chunkSize == (MEMLENGTH * sizeof(double))) {
        return 1;
    }
	return 0;
}

// Coalesce provided chunk with the next chunk if both are free
int coalesce(chunkheader* head) {
    chunkheader* next = &(getNextChunk(head)->header);

    if (next < (chunkheader*) (memory + MEMLENGTH)) {
         if (getAllocationStatus(head) == FREE && getAllocationStatus(next) == FREE) {
            setChunkSize(head, getChunkSize(head) + getChunkSize(next));
            setChunkSize(next, 0);
            return 1;
        }
    } 
    return 0;
}

void* mymalloc(size_t size, char* file, int line) {
    if (size == 0) {
        fprintf(stderr, "Error: Cannot allocate 0 bytes @ File: %s, Line: %d\n", file, line);
        return NULL;
    }
    size = REALIGN8(size);
    size_t neededSize = size + HEADER_SIZE;

    // Get first chunkheader
    chunkheader* start = (chunkheader*) memory;
    
    // Initialize memory
    size_t memSize = MEMLENGTH * sizeof(double);
    if (getAllocationStatus(start) == 0 && getChunkSize(start) == 0) {
        setChunkSize(start, memSize);
        setFree(start);
    }
    size_t currByte = 0;
    
    while (currByte < memSize) {
        size_t originalChunkSize = getChunkSize(start);
        if (getAllocationStatus(start) == FREE && originalChunkSize >= neededSize) {
            size_t remainingChunkSize = originalChunkSize - neededSize;

            // Allocate the chunk of needed size
            setChunkSize(start, neededSize);
            setAllocated(start);

            // If there is remaining space from the original size, update the next chunk's size
            if (remainingChunkSize > 0) {
                chunk* nextChunk = getNextChunk(start);
                setChunkSize(&(nextChunk->header), remainingChunkSize);
            }

            return getPayload(start);
        }
        else {
            // Iterate to the next available chunk
            currByte += getChunkSize(start);
            start = &(getNextChunk(start)->header);
        }
    }
    
    fprintf(stderr, "Error: insufficient memory @ File: %s, Line: %d\n", file, line);
    return NULL;
}

void myfree(void* ptr, char* file, int line) {
    // Check if the ptr is null - return an error message
    if (ptr == NULL) {
        fprintf(stderr, "Error: Cannot free NULL pointer @ File: %s, Line: %d\n", file, line);
        return;
    }

    // Get first chunkheader
    chunkheader* prev = NULL;
    chunkheader* start = (chunkheader*) memory;
    size_t memSize = MEMLENGTH * sizeof(double);
    size_t currByte = 0;
    int returnable = 0;
    
    while (currByte < memSize && !returnable) {
        if (getPayload(start) == (chunkpayload*) ptr) {
            // Check if we are freeing an object that is already free
            if (getAllocationStatus(start) == FREE) {
                fprintf(stderr, "Error: Cannot free pointer that was already free @ File: %s, Line: %d\n", file, line);
                return;
            }

            setFree(start);
            returnable = 1;
        }

        // Coalesce if the next chunk is free (start chunk with next chunk)
        coalesce(start);
        // Coalesce if the previous chunk is free (previous chunk with start chunk)
        if (prev != NULL && coalesce(prev)) {
            start = prev;
        }
        // If all chunks are free, everything should now be coalesced into prev chunk

        prev = start;
        currByte += getChunkSize(start);
        start = &(getNextChunk(start)->header);
    }
    if (returnable == 1) return;
    
    fprintf(stderr, "Error: Cannot free pointer that was not allocated @ File: %s, Line: %d\n", file, line);
    return;
}
