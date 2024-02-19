#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>


// Compile with -DREALMALLOC to use the real malloc() instead of mymalloc()
#ifndef REALMALLOC
#include "mymalloc.h"
#endif

#define NUM_RUNS 50

int getTime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    
    // tv.tv_sec returns time in seconds, so multiply by 1000 to get time in milliseconds
    return tv.tv_usec;
}

int runTest(int (*test)()) {

    double totalTime = 0;

    for (int i = 0; i < NUM_RUNS; i++) {
        int startTime = getTime();
        int result = test();
        int endTime = getTime();

        totalTime += endTime - startTime;

        printf("Run %d - Memory Cleared?: %d\n", i + 1, result);
    }

    double averageTime = (totalTime) / (NUM_RUNS);
    printf("Average Time: %f milliseconds\n", averageTime);

    return averageTime;

}

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
    // printf("Test 1 - Memory Cleared?: %d\n", test1());
    // printf("Test 2 - Memory Cleared?: %d\n", test2());
    // printf("Test 3 - Memory Cleared?: %d\n", test3());

    printf("Test 1:\n");
    runTest(test1);

    printf("\nTest 2:\n");
    runTest(test2);

    printf("\nTest 3:\n");
    runTest(test3);

    return 0;
}
