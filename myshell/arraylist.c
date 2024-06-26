#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "arraylist.h"

#ifndef DEBUG
#define DEBUG 0
#endif

arraylist_t* al_init(unsigned size)
{
    arraylist_t* L = malloc(sizeof(arraylist_t));
    if (L == NULL) {
        perror("Error allocating memory");
        return NULL;
    }
    L->data = malloc(size * sizeof(char*));
    if (L->data == NULL) {
        perror("Error allocating memory");
        free(L);
        return NULL;
    }
    L->length = 0;
    L->capacity = size;

    return L;
}

void al_destroy(arraylist_t* L)
{
    for (unsigned i = 0; i < L->length; i++) {
        free(L->data[i]);
    }
    free(L->data);
    free(L);
}

unsigned al_length(arraylist_t* L)
{
    return L->length;
}

char* al_get(arraylist_t* L, unsigned index)
{
    if (index >= L->length) {
        fprintf(stderr, "Index out of bounds\n");
        exit(EXIT_FAILURE);
    }

    return L->data[index];
}

void al_push(arraylist_t* L, char* item)
{
    if (L->length == L->capacity) {
        L->capacity *= 2;
        char** temp = realloc(L->data, L->capacity * sizeof(char*));
        if (!temp) { 
            // for our own purposes, we can decide how to handle this error
            // for more general code, it would be better to indicate failure to our caller
            fprintf(stderr, "Out of memory!\n");
            exit(EXIT_FAILURE);
        }

        L->data = temp;
        if (DEBUG) printf("Resized array to %u\n", L->capacity);
    }
    
    if (item == NULL) {
        L->data[L->length] = NULL;
    } else {
        L->data[L->length] = malloc(strlen(item) + 1);
        if (L->data[L->length] == NULL) {
            fprintf(stderr, "Out of memory!\n");
            exit(EXIT_FAILURE);
        }
        memcpy(L->data[L->length], item, strlen(item) + 1);
    }
    L->length++;
}


void al_insert(arraylist_t* L, int pos, char* item)
{
    if ((unsigned) pos > L->length || pos < 0) {
        fprintf(stderr, "Index out of bounds\n");
        exit(EXIT_FAILURE);
    }

    if (L->length == L->capacity) {
        L->capacity *= 2;
        char** temp = realloc(L->data, L->capacity * sizeof(char*));
        if (!temp) { 
            // for our own purposes, we can decide how to handle this error
            // for more general code, it would be better to indicate failure to our caller
            fprintf(stderr, "Out of memory!\n");
            exit(EXIT_FAILURE);
        }

        L->data = temp;
        if (DEBUG) printf("Resized array to %u\n", L->capacity);
    }

    char copy[strlen(item) + 1]; // we make a copy incase item is a string later in the arraylist
    if (item != NULL) {
        memcpy(copy, item, strlen(item) + 1);
    }
    
    for (unsigned i = L->length; i > (unsigned) pos; i--) {
        if (i == L->length) {
            L->data[i] = malloc(strlen(L->data[i - 1]) + 1);
        } else {
            L->data[i] = realloc(L->data[i], strlen(L->data[i - 1]) + 1);
        }
        if (L->data[i] == NULL) {
            fprintf(stderr, "Out of memory!\n");
            exit(EXIT_FAILURE);
        }
        memcpy(L->data[i], L->data[i - 1], strlen(L->data[i - 1]) + 1);
    }

    if (item == NULL) {
        L->data[pos] = NULL;
    } else {
        if ((unsigned) pos == L->length) {
            L->data[pos] = malloc(sizeof(copy));
        } else {
            L->data[pos] = realloc(L->data[pos], sizeof(copy));
        }
        if (L->data[pos] == NULL) {
            fprintf(stderr, "Out of memory!\n");
            exit(EXIT_FAILURE);
        }
        memcpy(L->data[pos], copy, strlen(item) + 1);
    }
    L->length++;
}

// returns 1 on success and writes popped item to dest
// returns 0 on failure (list empty)

int al_pop(arraylist_t* L, char** dest)
{
    if (L->length == 0) return 0;

    L->length--;
    if (dest != NULL) *dest = L->data[L->length];
    free(L->data[L->length]);

    return 1;
}

int al_remove(arraylist_t* L, int pos, char** dest) 
{
    if (L->length == 0 || pos < 0 || (unsigned) pos >= L->length) return 0;
    
    if (dest != NULL) *dest = L->data[pos];
    L->length--;
    for (unsigned i = pos; i < L->length; i++) {
        L->data[i] = realloc(L->data[i], strlen(L->data[i + 1]) + 1);
        if (L->data[i] == NULL) {
            fprintf(stderr, "Out of memory!\n");
            exit(EXIT_FAILURE);
        }
        memcpy(L->data[i], L->data[i + 1], strlen(L->data[i + 1]) + 1);
    }
    free(L->data[L->length]);
    
    return 1;
}

void al_set(arraylist_t* L, int pos, char* item)
{
    if ((unsigned) pos >= L->length || pos < 0) {
        fprintf(stderr, "Index out of bounds\n");
        exit(EXIT_FAILURE);
    } else {
        if (item == L->data[pos]) return;
        if (item == NULL) {
            free(L->data[pos]);
            L->data[pos] = NULL;
            return;
        }
        L->data[pos] = realloc(L->data[pos], strlen(item) + 1);
        if (L->data[pos] == NULL) {
            fprintf(stderr, "Out of memory!\n");
            exit(EXIT_FAILURE);
        }
        memcpy(L->data[pos], item, strlen(item) + 1);
    }
}