#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


// Compile with -DREALMALLOC to use the real malloc() instead of mymalloc()
#ifndef REALMALLOC
#include "mymalloc.h"
#endif

#define MEMSIZE 4096
#define HEADERSIZE 16
#define OBJECTS 64
#define OBJSIZE (MEMSIZE / OBJECTS - HEADERSIZE)

int main()// (int argc, char **argv)
{
	char *obj[OBJECTS];
	int i, j, errors = 0;
	
	// fill memory with objects
	for (i = 0; i < OBJECTS; i++) {
		obj[i] = malloc(OBJSIZE);
	}
	
	// fill each object with distinct bytes
	for (i = 0; i < OBJECTS; i++) {
		memset(obj[i], i, OBJSIZE);
	}
	
	// check that all objects contain the correct bytes
	for (i = 0; i < OBJECTS; i++) {
		for (j = 0; j < OBJSIZE; j++) {
			if (obj[i][j] != i) {
				errors++;
				printf("Object %d byte %d incorrect: %d\n", i, j, obj[i][j]);
			}
		}
	}
	
	printf("%d incorrect bytes\n", errors);

	for (i = 0; i< OBJECTS; i++) {
		free(obj[i]);
	}

	int* arr2[170];
	for(int i = 0; i < 32; i++){
		arr2[i] = malloc(108);
		*arr2[i] = i;
	}

	for(int i = 0; i < 32; i++){
		if(*arr2[i] != i){
			printf("Test failed, pointers overlapped.");
		}
	}

	printf("Memory is currently full - should throw max memory error after calling malloc one more time:\n");
	malloc(4);

    printf("Going to call free in middle of a 32 byte chunk of allocated data - should throw error for bad pointer\n");
    free(((int*) arr2[0]) + 1);

	for(int i = 0; i < 32; i++){
		free(arr2[i]);
	}

	printf("Freed all pointers, freeing first pointer again should throw double free error:\n");
	free(arr2[0]);

	printf("Calling malloc on 0 bytes should throw an error:\n");
	malloc(0);
	
	return EXIT_SUCCESS;
}
