typedef struct {
    char** data;
    unsigned length;
    unsigned capacity;
} arraylist_t;

void al_init(arraylist_t*, unsigned);
void al_destroy(arraylist_t*);

unsigned al_length(arraylist_t*);

char* al_get(arraylist_t* L, unsigned index);
void al_push(arraylist_t*, char*);
void al_push_at_pos(arraylist_t* L, int pos, char* item);
int al_pop(arraylist_t*, char**);