#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>


// Compile with -DREALMALLOC to use the real malloc() instead of mymalloc()
#ifndef REALMALLOC
#include "mymalloc.h"
#endif

int test1() {
    for(int i = 0; i < 120; i++) {
        char *ptr = malloc(1);  
        free(ptr);
    }

    return getMemoryStatus();
}

int test2() {
    char* objects[120];

    for(int i = 0; i < 120; i++) {
        objects[i] = malloc(1); 
    }

    for(int i = 0; i < 120; i++) {
        free(objects[i]);  
    }

    return getMemoryStatus();
}

int test3() {
    char* objects[120];
    int index = 0;

    while (index < 120) {
        int randInt = rand() % 2;
        if (randInt == 0) {
            objects[index] = malloc(1);
            index++;
        } else {
            if (index > 0 && objects[index-1] != NULL) {
                free(objects[index-1]);
                objects[index-1] = NULL;
            }
        }
    }

    for (int i = 0; i < 120; i++) {
        if (objects[i] != NULL) {
            free(objects[i]);
        }
    }

    return getMemoryStatus();
}

int main() {
    printf("Test 1 - Memory Cleared?: %d\n", test1());
    printf("Test 2 - Memory Cleared?: %d\n", test2());
    printf("Test 3 - Memory Cleared?: %d\n", test3());
    return 0;
}
