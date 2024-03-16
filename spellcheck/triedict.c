#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

typedef struct trie_node trie;

struct trie_node {
    char letter; // letter of the node (uppercase can only be uppercase, lowercase can be either)
    int isWord; // 1 if word, 0 if not
    trie* children[26]; 
};

trie* dict;

trie* get_dict() {
    return dict;
}

int init_trie() {
    dict = malloc(sizeof(trie));
    if (dict == NULL) {
        perror("Error allocating memory");
        return -1;
    }
    dict->letter = '\0';
    dict->isWord = 0;
    for (int ii = 0; ii < 26; ii++) {
        dict->children[ii] = NULL;
    }
    return 0;
}

int add_word_to_trie(char* word) {
    trie* node = get_dict();
    for (int ii = 0; ii < strlen(word); ii++) {
        char letter = word[ii];
        int index = letter - (letter >= 'A' && letter <= 'Z' ? 'A' : 'a');
        
        if (node->children[index] == NULL) {
            trie* new_node = malloc(sizeof(trie));
            if (new_node == NULL) {
                perror("Error allocating memory");
                return -1;
            }
            new_node->letter = letter;
            new_node->isWord = 0;
            for (int jj = 0; jj < 26; jj++) {
                new_node->children[jj] = NULL;
            }
            node->children[index] = new_node;
        }
        node = node->children[index];
    }

    return 0;
}
