#ifndef ARRAYLIST_H
#define ARRAYLIST_H

typedef struct {
    char** data;
    unsigned length;
    unsigned capacity;
} arraylist_t;

arraylist_t* al_init(unsigned size);
void al_destroy(arraylist_t* L);

unsigned al_length(arraylist_t* L);

char* al_get(arraylist_t* L, unsigned index);
void al_push(arraylist_t* L, char* item);
void al_put(arraylist_t* L, int pos, char* item);
int al_pop(arraylist_t* L, char** dest);
int al_remove(arraylist_t* L, int pos, char** dest);
void al_set(arraylist_t* L, int pos, char* item);

#endif