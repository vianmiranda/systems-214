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

enum allocation_status {
    FREE,
    TAKEN
};

/*--------------------------------------------------*/
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

// Marks as either 0 for free
void setFree(chunkheader* head) {
    head->is_allocated = FREE;
}

// Marks as either 1 for taken
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
    size_t memory = (MEMLENGTH * sizeof(double) - HEADER_SIZE);
    if (getAllocationStatus(start) == 0 && getChunkSize(start) == 0) {
        setChunkSize(start, memory);
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
                setChunkSize(nextChunk, remainingChunkSize - HEADER_SIZE);
            }

            return getPayload(start);
        }
        else {
            // Iterate to the next available chunk
            currByte += getChunkSize(start);
            start = getNextChunk(start);
        }
    }
    
    fprintf(stderr, "Error: insufficient memory @ File: %s, Line: %d\n", file, line);
    return NULL;
}


// void* mymalloc(size_t size, char* file, int line) {
//     if (size == 0) {
//         fprintf(stderr, "Cannot allocate 0 bytes @ File: %s, Line: %d\n", file, line);
//         return NULL;
//     }
//     size = ROUNDUP8(size);

//     int* head = (int*) memory;
//     void* res;

//     //initialize
//     if (getChunkSize(head) == 0 && isChunkFree(head)) {
//       *head = (MEM_SIZE*8);
//       setFree(head);
//     }

//     int byte = 0;

//     while(byte < MEM_SIZE*8) {

//         int isFree = isChunkFree(head);

//         /*if size < current chunksize and is free, allocate

//          */
//         if (size <= getChunkSize(head)-HEADER_SIZE && isChunkFree(head)) {

//           //keep track of the size of the original chunk before allocation
//           int originalchunksize = getChunkSize(head);

//           //update current chunk to match the size requested and mark as allocated
//           setChunkSize(head, size);
//           markAsAllocated(head);

//           //set pointer to payload (+2 because first 2 integers (8 bytes) is used for size(4 bytes) and free status (4 bytes))
//           res = (void*)(head + 2);

//           //if there is still space after allocation adjust space of next chunk
//           // originalchunksize - getChunkSize(head) - 8 --> subtract the size of the allocated portion and the
//             // header from the original chunk size
//           if (originalchunksize > getChunkSize(head)) {
//             setChunkSize(getNextChunk(head), originalchunksize - getChunkSize(head) - 8);
//             setFree(getNextChunk(head));
//           } 

//           return res;
//         } else {
//           //iterate to the next available chunk
//           byte += getChunkSize(head);
//           head = getNextChunk(head);
//         }

//         //otherwise there is not enough memory
//         fprintf(stderr, "Error: no more memory @ File: %s, Line: %d\n", file, line);
//         return NULL;


//     }

//       //print error msg

    
// }



// /*
// errors to detect:
// 1. freeing a chunk that was not malloced
// 2. not freeing at the start of the chunk
// 3. calling free a 2nd time on the same pointer
// */
// void myfree(void *ptr, char *file, int line) {
// }



/*

void* mymalloc(size_t size, char *file, int line) {

  //check if size == 0 and print error msg
  if (size == 0) {
    fprintf(stderr, "Can't allocate 0 bytes. File: %s, Line: %d\n", file, line);
    return NULL;
  }

  //round to nearest multiple of 8
  size = ROUNDUP8(size);

  //initialize memory
  if (free_chunk_list == NULL) {
    initialize_memory();
  }

  chunkheader* current = free_chunk_list;
  chunkheader* prev = NULL;

  while (current != NULL) {
    //check if chunk is large enough (1)
    if (!current->is_allocated && current->size >= size) {
      //if there is enough space, split chunk (2)
      if (isLargeEnough(current, size)) {
        //adding sizeof(chunkheader) because metadata needs space in the chunk
        chunkheader* nextchunk = (chunkheader*)((char*)current + size + sizeof(chunkheader));
        nextchunk->size = current->size - size - sizeof(chunkheader);
        nextchunk->is_allocated = 0; //this is space that is "free"
        nextchunk->next = current->next;
        current->size = size;
        current->next = nextchunk;
      }
      //mark chunk as allocated
      current->is_allocated = 1;

      //remove chunk from free_chunk_list since it is now allocated
      if (prev == NULL) {
        free_chunk_list = current->next;
      } else {
        prev->next = current->next;
      }

      //return a pointer to payload
      return (void*)((char*)current + sizeof(chunkheader)); //use char to manipulate bytes, essentially payload is immediately after the space allocated for chunkheader(metadata)
    }

    //move to next chunk
    prev = current;
    current = current->next;

  }

  //at this point, no chunk was found, so we return NULL
  return NULL;

}




*/



// //create a struct for the chunk header. this will act as a LL to move from one chunk to next available chunk
// typedef struct chunkheader {
//   size_t size;
//   int is_allocated; //int can be represented as true/false
//   struct chunckheader* next;
// } chunkheader;

// //initially, list of free chunks is null
// chunkheader* free_chunk_list = NULL;


// //method to initialize memory and LL
// void initialize_memory() {
//   free_chunk_list = (chunkheader*)memory;
//   free_chunk_list->size = MEM_SIZE - METADATA_SIZE;
//   free_chunk_list->is_allocated = 0;
//   free_chunk_list->next = NULL;
// }

// /*
// instructions:
//   1. search for free chunk of memory containing >= requested bytes to be allocated
//   2. if chunk is large enough, you may divide the chunk into 2, 1st large enough for request
//      (and return pointer to it) and 2nd remains free
// */

// //checks if chunk is large enough and not already allocated
// int isLargeEnough(chunkheader* curr, size_t size) {
//   if (!curr->is_allocated && curr->size >= size) {
//     return 1;
//   }
//   return 0;
// }