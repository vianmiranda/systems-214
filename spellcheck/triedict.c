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

trie* dict;

/**
 * @return the dict node of the trie
*/
trie* get_dict() {
    return dict;
}

/**
 * Initializes the dict nodeof the trie
 * 
 * @return 0 if successful, -1 if not
*/
int init_trie() {
    dict = malloc(sizeof(trie));
    if (dict == NULL) {
        perror("Error allocating memory");
        return -1;
    }
    dict->letter = '\0';
    dict->isWord = 0;
    for (int ii = 0; ii < NUM_LETTERS; ii++) {
        dict->children[ii] = NULL;
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
    for (int ii = 0; ii < strlen(word); ii++) {
        char letter = word[ii];
        int index = char_to_index(letter);
        if (index == -1) {
            return -1;
        }
        
        if (node->children[index] == NULL) {
            trie* new_node = malloc(sizeof(trie));
            if (new_node == NULL) {
                perror("Error allocating memory");
                return -1;
            }
            new_node->letter = letter;
            new_node->isWord = 0;
            for (int jj = 0; jj < NUM_LETTERS; jj++) {
                new_node->children[jj] = NULL;
            }
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
    for (int ii = 0; ii < strlen(word); ii++) {
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