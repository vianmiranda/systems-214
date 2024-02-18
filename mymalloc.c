#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h" //reference the related header file

/*
  chunk:
  1. header (metadata) - informs runtime system about chunk size and whether the chunk is allocated
  2. payload (data) - the actual object/data, a contiguous sequence of bytes

*/

//makes sure size is rounded to nearest multiple of 8
#define ROUNDUP8(x) (((x)+7) & (-8))

// define header size (int for size and int for allocation status 1 or 0)
#define HEADER_SIZE sizeof(int)*2

//define memory block 
static double memory[MEM_SIZE];

int getChunkSize(void* head) {
  return *((size_t*)head);
}

int isChunkFree(void* head) {
  return *(((int*)head) + 1) == 0;
}

void markAsAllocated(void* head) {
  *(((int*)head) + 1) = 1;
}

void setChunkSize(void* head, size_t size) {
  *((int*) head) = size + 8;
}

int* getNextChunk(void* head) {
  return ((int*)head) + *((int*)head) / 4;
}

void setFree(void* head) {
  return *((int*)head + 1) = 0;
}



void* mymalloc(size_t size, char* file, int line) {
    if (size == 0) {
        fprintf(stderr, "Cannot allocate 0 bytes @ File: %s, Line: %d\n", file, line);
        return NULL;
    }
    size = ROUNDUP8(size);

    int* head = (int*) memory;
    void* res;

    //initialize
    if (getChunkSize(head) == 0 && isChunkFree(head)) {
      *head = (MEM_SIZE*8);
      setFree(head);
    }

    int byte = 0;

    while(byte < MEM_SIZE*8) {

        int isFree = isChunkFree(head);

        /*if size < current chunksize and is free, allocate

         */
        if (size <= getChunkSize(head)-HEADER_SIZE && isChunkFree(head)) {

          //keep track of the size of the original chunk before allocation
          int originalchunksize = getChunkSize(head);

          //update current chunk to match the size requested and mark as allocated
          setChunkSize(head, size);
          markAsAllocated(head);

          //set pointer to payload (+2 because first 2 integers (8 bytes) is used for size(4 bytes) and free status (4 bytes))
          res = (void*)(head + 2);

          //if there is still space after allocation adjust space of next chunk
          // originalchunksize - getChunkSize(head) - 8 --> subtract the size of the allocated portion and the
            // header from the original chunk size
          if (originalchunksize > getChunkSize(head)) {
            setChunkSize(getNextChunk(head), originalchunksize - getChunkSize(head) - 8);
            setFree(getNextChunk(head));
          } 

          return res;
        } else {
          //iterate to the next available chunk
          byte += getChunkSize(head);
          head = getNextChunk(head);
        }

        //otherwise there is not enough memory
        fprintf(stderr, "Error: no more memory @ File: %s, Line: %d\n", file, line);
        return NULL;


    }

      //print error msg

    
}



/*
errors to detect:
1. freeing a chunk that was not malloced
2. not freeing at the start of the chunk
3. calling free a 2nd time on the same pointer
*/
void myfree(void *ptr, char *file, int line) {
}



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