#ifndef _MYMALLOC_H
#define _MYMALLOC_H

#define malloc(s)   mymalloc(s, __FILE__, __LINE__)
#define free(p)     myfree(p, __FILE__, __LINE__)

void *mymalloc(size_t size, char *file, int line);
void myfree(void *ptr, char *file, int line);
int getMemoryStatus();

// Makes sure size is rounded to nearest multiple of 8
#define REALIGN8(x) (((x)+7) & (~7))

// Define metadata size 
// 1. int for size 
// 2. int for allocation status, either 1 for taken or 0 for free
#define HEADER_SIZE (sizeof(int)*2)

// Amount of memory allocation for global array
#define MEMLENGTH  512

#endif
