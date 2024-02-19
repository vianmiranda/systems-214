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
    
    return tv.tv_usec;
}

int runTest(int (*test)()) {

    double totalTime = 0;

    for (int i = 0; i < NUM_RUNS; i++) {
        int startTime = getTime();
        int result = test();
        int endTime = getTime();

        if (!result) {
            return 0;
        }

        double time = endTime - startTime;
        totalTime += time;

        printf("Run %d - Memory Cleared?: %d; Runtime: %f\n", i + 1, result, time);
    }

    double averageTime = (totalTime) / (NUM_RUNS);
    printf("Average Time: %f milliseconds\n", averageTime);

    return 1;
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
    int passed = 0;
    printf("Test 1:\n");
    passed += runTest(test1);

    printf("\nTest 2:\n");
    passed += runTest(test2);

    printf("\nTest 3:\n");
    passed += runTest(test3);

    printf("\nTests Passed: %d/5\n", passed);

    return 0;
}
