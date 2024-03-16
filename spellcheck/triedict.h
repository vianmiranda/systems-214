typedef struct trie_node trie;

trie* get_dict();
int init_trie();
int add_word_to_trie(char* word);
int check_word_in_trie(char* word);