typedef struct trie_node trie;

trie* get_dict();
int init_trie();
int free_trie(trie* node);
int add_word_to_trie(char* word);
int check_word_in_trie(char* word);

#define free_trie() free_trie(get_dict())
