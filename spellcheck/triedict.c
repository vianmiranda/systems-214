#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#define NUM_LETTERS 256

typedef struct trie_node trie;

struct trie_node {
    char letter; // letter of the node in the trie
    int isWord; // 1 if word, 0 if not
    trie* children[NUM_LETTERS]; 
};

trie* dict = NULL;

/**
 * @return the dict node of the trie
*/
trie* get_dict() {
    return dict;
}

/**
 * Recursively frees all trie_nodes
*/
void free_trie(trie* node) {
    for (int ii = 0; ii < NUM_LETTERS; ii++) {
        // printf("%d: Freeing %p\n", ii, node->children[ii]);
        if (node->children[ii] != NULL) {
            free_trie(node->children[ii]);
        }
    }
    free(node);
}

/**
 * Initializes the dict nodeof the trie
 * 
 * @return 0 if successful, -1 if not
*/
int init_trie() {
    dict = calloc(1, sizeof(trie));
    if (dict == NULL) {
        perror("Error allocating memory");
        return -1;
    }
    return 0;
}

/**
 * Get the current letter and store it as an index
 *
 * @returns the index (0-255), -1 if error
*/
int char_to_index(char letter) {
    int index = letter + 128;
    if (index < 0 || index > NUM_LETTERS) {
        fprintf(stderr, "Invalid character in word: %c\n", letter);
        return -1;
    }
    return index;
}

/**
 * Adds a word to the trie
 * 
 * @returns 0 if successful, -1 if not
*/
int add_word_to_trie(char* word) {
    trie* node = get_dict();
    for (int ii = 0; ii < (int) strlen(word); ii++) {
        char letter = word[ii];
        int index = char_to_index(letter);
        if (index == -1) {
            return -1;
        }
        
        if (node->children[index] == NULL) {
            trie* new_node = calloc(1, sizeof(trie));
            if (new_node == NULL) {
                perror("Error allocating memory");
                free(new_node);
                return -1;
            }
            new_node->letter = letter;
            node->children[index] = new_node;
        }
        node = node->children[index];
    }
    node->isWord = 1;

    return 0;
}

/**
 * Checks if a word is in the trie
 * 
 * @returns 1 if word is in trie, 0 if not, -1 if error
*/
int check_word_in_trie(char* word) {
    trie* node = get_dict();
    for (int ii = 0; ii < (int) strlen(word); ii++) {
        char letter = word[ii];
        int index = char_to_index(letter);
        if (index == -1) {
            return -1;
        }

        if (node->children[index] == NULL) {
            // word is automatically not in trie
            return 0;
        }
        node = node->children[index];
    }

    // we are at the last node (last character in the word), so check its "isWord"
    return node->isWord;
}