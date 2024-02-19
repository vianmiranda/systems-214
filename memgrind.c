#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


// Compile with -DREALMALLOC to use the real malloc() instead of mymalloc()
#ifndef REALMALLOC
#include "mymalloc.h"
#endif

void test1() {
    for(int i = 0; i < 120; i++) {
        char *ptr = malloc(1);  
        free(ptr);
        printf("ptrArray[%d]: %p\n", i, ptr);
    }
    printf("Memory Cleared?: %d\n", getMemoryStatus()); 
}

void test2() {
    char* objects[120];

    for(int i = 0; i < 120; i++) {
        objects[i] = malloc(1); 
    }

    for(int i = 0; i < 120; i++) {
        free(objects[i]);  
    }

    printf("Memory Cleared?: %d\n", getMemoryStatus()); 
}

void test3() {
    
}

int main() {
    test1();
    test2();
    return 0;
}
