#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h" //reference the related header file

/*
  chunk:
  1. header (metadata) - informs runtime system about chunk size and whether the chunk is allocated
  2. payload (data) - the actual object/data, a contiguous sequence of bytes

*/

// Makes sure size is rounded to nearest multiple of 8
#define ROUNDUP8(x) (((x)+7) & (~7))

// Define metadata size 
// 1. int for size 
// 2. int for allocation status, either 1 for taken or 0 for free
#define HEADER_SIZE (sizeof(int)*2)

// Allocated memory from global array
#define MEMLENGTH  512
static double memory[MEMLENGTH];

// 0 = free, 1 = taken
enum allocation_status {
    FREE,
    TAKEN
};

typedef struct {
    size_t size;
    int is_allocated;
    // chunkheader* next;
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
    head->size = size + HEADER_SIZE;
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
    return (chunkpayload*)(((char*)head) + HEADER_SIZE);
}

// Returns the pointer to the next chunk
chunk* getNextChunk(chunkheader* head) {
    return (chunk*)(((char*)head) + getChunkSize(head));
}


void* mymalloc(size_t size, char* file, int line) {
    if (size == 0) {
        fprintf(stderr, "Error: Cannot allocate 0 bytes @ File: %s, Line: %d\n", file, line);
        return NULL;
    }
    size = ROUNDUP8(size);

    // Get first chunkheader
    chunkheader* start = (chunkheader*) memory;
    
    // Initialize memory
    size_t memory = MEMLENGTH * sizeof(double);
    if (getAllocationStatus(start) == 0 && getChunkSize(start) == 0) {
        setChunkSize(start, memory - HEADER_SIZE);
        setFree(start);
    }
    int currByte = 0;
    size_t neededSize = size + HEADER_SIZE;

    
    while (currByte < memory) {
        size_t originalChunkSize = getChunkSize(start);
        if (getAllocationStatus(start) == FREE && originalChunkSize >= neededSize) {
            size_t newChunkSize = originalChunkSize - neededSize;
            size_t remainingChunkSize = originalChunkSize - newChunkSize;

            // Allocate the chunk of needed size
            setChunkSize(start, size);
            setAllocated(start);

            // If there is remaining space from the original size, update the next chunk's size
            if (remainingChunkSize > 0) {
                chunk* nextChunk = getNextChunk(start);
                setChunkSize(&(nextChunk->header), remainingChunkSize - HEADER_SIZE);
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
    chunkheader* start = (chunkheader*) memory;
    size_t memory = MEMLENGTH * sizeof(double);
    int currByte = 0;
    
    while (currByte < memory) {
        if (start == (chunkheader*) ptr) {

            // Check if we are freeing an object that is already free
            if (getAllocationStatus(start) == FREE) {
                fprintf(stderr, "Error: Cannot free pointer that was already free @ File: %s, Line: %d\n", file, line);
                return;
            }

            setFree(start);
            return;
        }
        chunkheader* prev = start;
        currByte += getChunkSize(start);
        start = &(getNextChunk(start)->header);

        // Coalesce neighboring free chunks
        if (getAllocationStatus(prev) == FREE && getAllocationStatus(start) == FREE) {
            setChunkSize(prev, getChunkSize(prev) + getChunkSize(start) - HEADER_SIZE);
            setChunkSize(start, 0);
        }
    }
    
    fprintf(stderr, "Error: Cannot free pointer that was not allocated @ File: %s, Line: %d\n", file, line);
    return;
}
