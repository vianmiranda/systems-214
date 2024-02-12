#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h" //reference the related header file


//define memory block 
static double memory[MEM_SIZE];



/*
size_t = size of requested memory block
file = the source file
line = line number of the application


chunk:
  1. header (metadata) - informs runtime system about chunk size and whether the chunk is allocated
  2. payload (data) - the actual object/data, a contiguous sequence of bytes

*/

//create a struct for the chunk header. this will act as a LL to move from one chunk to next available chunk

typedef struct chunkheader {
  size_t size;
  int is_allocated; //int can be represented as true/false
  struct chunckheader* next;
} chunkheader;

//initially, list of free chunks is null
chunkheader* free_chunk_list = NULL;

/*
instructions:
  1. search for free chunk of memory containing >= requested bytes to be allocated
  2. if chunk is large enough, you may divide the chunk into 2, 1st large enough for request
     (and return pointer to it) and 2nd remains free

*/
void* mymalloc(size_t size, char *file, int line) {
  chunkheader* current = free_chunk_list;
  chunkheader* prev = NULL;

  while (current != NULL) {
    //check if chunk is large enough (1)
    if (!current->is_allocated && current->size >= size) {
      //if there is enough space, split chunk (2)
      if (current->size > size + sizeof(chunkheader)) {
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



/*
errors to detect:
1. freeing a chunk that was not malloced
2. not freeing at the start of the chunk
3. calling free a 2nd time on the same pointer
*/
void myfree(void *ptr, char *file, int line) {

}

