#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "triedict.h"

#define BUFFER_SIZE 128

typedef struct {
    char** variations;
    int numVariations;
} cleanText;

int SUCCESS = 1;

/**
 * Create variations of a word based on the following rules:
 * 1. If the word is all lowercase, then create variations for all lowercase, proper case, and all uppercase
 * 2. If the word is proper case, then create variations for proper case and all uppercase
 * 3. If the word is all uppercase, then create variations for all uppercase
 * 4. If the word is none of the above, create an exact match case and all uppercase
 * 
 * @param word: The word to create variations for
 * @return cleanText*: A struct containing the valid variations of the word
*/
cleanText* create_variations(char* word) {
    int exactMatch = 1, allLowercase = 0, properCase = 0, allUppercase = 0; 

    int numUppercase = 0;
    int wordLen = strlen(word);
    int ignorePunctuation = wordLen;
    for (int ii = 0; ii < wordLen; ii++) {
        if (isupper(word[ii])) {
            numUppercase += 1;
            if (ii == 0) {
                properCase = 1;
            } else {
                properCase = 0;
            }
        } else if (!isalpha(word[ii])) {
            ignorePunctuation -= 1;
        }
    }
    
    if (numUppercase == 0) {
        // current word is all lowercase, therefore create variations for all lowercase, proper case, and all uppercase
        exactMatch = 0;
        allLowercase = 1;
        properCase = 1;
        allUppercase = 1;
    }
    if (numUppercase > 0) {
        // current word has at least 1 uppercase letter, so create variations for all uppercase and exact match
        allUppercase = 1;
    } 
    if (numUppercase == ignorePunctuation) {
        // current word is all uppercase, so create variations for all uppercase
        allUppercase = 1;
        exactMatch = 0;
    }

    cleanText* clean = malloc(sizeof(cleanText));
    if (clean == NULL) {
        perror("Error allocating memory");
        return NULL;
    }
    if (ignorePunctuation <= 0) {
        clean->numVariations = 0;
        clean->variations = NULL;
        return clean;
    }

    char parsed[wordLen + 1];
    strncpy(parsed, word, wordLen);
    parsed[wordLen] = '\0';
    wordLen++;

    int numVariations = exactMatch + allLowercase + properCase + allUppercase;
    clean->numVariations = numVariations;

    char** res = malloc(numVariations * sizeof(char*));
    if (res == NULL) {
        perror("Error allocating memory");
        free(clean);
        return NULL;
    }
    for (int ii = 0; ii < numVariations; ii++) {
        res[ii] = malloc(wordLen * sizeof(char));
        if (res[ii] == NULL) {
            perror("Error allocating memory");
            for (int jj = 0; jj < ii; jj++) {
                free(res[jj]);
            }
            free(res);
            free(clean);
            return NULL;
        }
    }

    int iterator = 0;
    if (exactMatch) {
        // no need to change anything
        res[iterator] = strncpy(res[iterator], parsed, wordLen);
        iterator++;
    }
    if (allLowercase) {
        // change all to lowercase
        for (int ii = 0; ii < wordLen; ii++) {
            if (isupper(parsed[ii])) {
                parsed[ii] = parsed[ii] + 32;
            }
        }
        res[iterator] = strncpy(res[iterator], parsed, wordLen);
        iterator++;
    } if (properCase) {
        // change first to uppercase
        if (islower(parsed[0])) {
            parsed[0] = parsed[0] - 32;
        }
        for (int ii = 1; ii < wordLen; ii++) {
            if (isupper(parsed[ii])) {
                parsed[ii] = parsed[ii] + 32;
            }
        }
        res[iterator] = strncpy(res[iterator], parsed, wordLen);
        iterator++;
    } if (allUppercase) {
        // change all to uppercase
        for (int ii = 0; ii < wordLen; ii++) {
            if (islower(parsed[ii])) {
                parsed[ii] = parsed[ii] - 32;
            }
        }
        res[iterator] = strncpy(res[iterator], parsed, wordLen);
    }
    clean->variations = res;

    return clean;
}

/**
 * Create trie dictionary of all words from a given file. 
 * Each word should be on its own line.
 * 
 * @param fd: The file descriptor of the file to read from
 * @return int: 0 if successful, -1 if an error occurred
*/
int create_dict(int fd) {
    // read from file and create trie
    char buffer[BUFFER_SIZE];
    if (buffer == NULL) {
        perror("Error allocating memory");
        return -1;
    }

    char word[BUFFER_SIZE];
    ssize_t jj = 0, bytesRead = 0;
    while ((bytesRead = read(fd, buffer, BUFFER_SIZE)) >= 0) {
        for (ssize_t ii = 0; ii < bytesRead || (bytesRead == 0 && jj != 0); ii++) {
            if (buffer[ii] == '\n' || (bytesRead == 0 && jj != 0)) {
                word[jj] = '\0';
                jj = 0;

                cleanText* wordVariations = create_variations(word);
                if (wordVariations == NULL) {
                    memset(word, 0, BUFFER_SIZE);
                    return -1;
                } else if (wordVariations->numVariations == 0) {
                    free(wordVariations);
                    memset(word, 0, BUFFER_SIZE);
                    continue;
                }

                for (int kk = 0; kk < wordVariations->numVariations; kk++) {
                    if (add_word_to_trie(wordVariations->variations[kk]) == -1) {
                        return -1;
                    }
                    free(wordVariations->variations[kk]);
                }
                free(wordVariations->variations);
                free(wordVariations);


                memset(word, 0, BUFFER_SIZE);
                continue;
            }
            word[jj] = buffer[ii];
            jj++;
        }
        if (bytesRead == 0) {
            break;
        }
    }
    if (bytesRead == -1) {
        perror("Error reading from file");
        return -1;
    }

    return 0;
}

/**
 * Clean a word by removing leading and trailing punctuation
 * 
 * @param word: The word to clean
 * @return cleanText*: A struct containing the cleaned version of the word
*/
cleanText* clean_text(char* word) {
    int trailingPunctuation; // index of the last alpha character
    for (trailingPunctuation = strlen(word) - 1; trailingPunctuation >= 0; trailingPunctuation--) {
        if (isalpha(word[trailingPunctuation])) {
            break;
        }
    }

    int leadingPunctuation; // index of the first alpha character
    for (leadingPunctuation = 0; leadingPunctuation < (int) strlen(word); leadingPunctuation++) {
        if (!(word[leadingPunctuation] == '[' 
        || word[leadingPunctuation] == '{' 
        || word[leadingPunctuation] == '(' 
        || word[leadingPunctuation] == '\"' 
        || word[leadingPunctuation] == '\'')) {
            break;
        } 
    }

    int wordLen = trailingPunctuation - leadingPunctuation + 1;

    cleanText* clean = malloc(sizeof(cleanText));

    if (wordLen <= 0) {
        clean->numVariations = 0;
        clean->variations = NULL;
        return clean;
    }

    char** res = malloc(1 * sizeof(char*));
    if (res == NULL) {
        perror("Error allocating memory");
        free(clean);
        return NULL;
    }

    res[0] = malloc((wordLen + 1) * sizeof(char));
    if (res[0] == NULL) {
        perror("Error allocating memory");
        free(clean);
        free(res);
        return NULL;
    }

    strncpy(res[0], word + leadingPunctuation, wordLen);
    res[0][wordLen] = '\0';

    clean->numVariations = 1;
    clean->variations = res;

    return clean;
}


/** 
 * Check each word in the hyphenated word. 
 * 
 * @param word: The hyphenated word to check
 * @return int: 0 if the word is not in the trie, 1 if the word is in the trie, -1 if an error occurred
*/
int handle_hyphenated_word(char *word) {
    char *temp = malloc(strlen(word) + 1);
    memcpy(temp, word, strlen(word) + 1);
    char *token = strtok(temp, "-");
    while (token != NULL) {
        if (check_word_in_trie(token) == 0) {
            free(temp);
            return 0;
        }
        token = strtok(NULL, "-");
    }
    free(temp);
    return 1;
}

/**
 * Check each word in a given file against the the provided dictionary
 * 
 * @param fd: The file descriptor of the file to read from
 * @param file_name: The name of the file to check
 * @return int: 0 if successful, -1 if an error occurred
*/
int check_text(int fd, char* file_name) {
    // read from file and check against trie1
    char buffer[BUFFER_SIZE];
    if (buffer == NULL) {
        perror("Error allocating memory");
        return -1;
    }

    int prevWhitespace = 1;
    int line_number = 1, col_number = 0, saved_col_number;
    int hyphen_present = 0;
    char word[BUFFER_SIZE];
    ssize_t jj = 0, bytesRead = 0;
    while ((bytesRead = read(fd, buffer, BUFFER_SIZE)) >= 0) {
        for (ssize_t ii = 0; ii < bytesRead || (bytesRead == 0 && jj != 0); ii++) {
            if (buffer[ii] == '\n') {
                // increment line_number and reset col_number
                line_number++;
                col_number = 0;
            } else {
                // increment col_number
                col_number++;
            }

            if (buffer[ii] == '-') {
                hyphen_present = 1;
            }

            if (!prevWhitespace && (isspace(buffer[ii]) || (bytesRead == 0 && jj != 0))) { 
                // If the previous character was not whitespace and the current character is whitespace, then we have a word
                word[jj] = '\0';
                jj = 0;
                prevWhitespace = 1;



                // Otherwise, word is normal. Clean leading and trailing punctuation and then check against trie
                cleanText* cleanWord = clean_text(word);
                if (cleanWord == NULL) {
                    return -1;
                } else if (cleanWord->numVariations == 0) {
                    free(cleanWord);
                    memset(word, 0, BUFFER_SIZE);
                    continue;
                }

                // Check if the word contains a hyphen
                if (hyphen_present) {
                    int hyphen_result = handle_hyphenated_word(cleanWord->variations[0]);
                    free(cleanWord->variations[0]);
                    free(cleanWord->variations);
                    free(cleanWord);
                    if (hyphen_result == -1) {
                        return -1;
                    } else if (hyphen_result == 0) {
                        SUCCESS = 0;
                        fprintf(stderr, "%s (%d, %d): %s\n", file_name, (line_number - (buffer[ii] == '\n' ? 1 : 0)), saved_col_number, word);   
                    }
                    hyphen_present = 0;
                    continue;
                }

                if (check_word_in_trie(cleanWord->variations[0]) == 0) {
                    SUCCESS = 0;
                    fprintf(stderr, "%s (%d, %d): %s\n", file_name, (line_number - (buffer[ii] == '\n' ? 1 : 0)), saved_col_number, word);
                }
                free(cleanWord->variations[0]);
                free(cleanWord->variations);
                free(cleanWord);

                memset(word, 0, BUFFER_SIZE);
                continue;
            } else if (prevWhitespace && !isspace(buffer[ii])) {
                // If the previous character was whitespace and the current character is not whitespace, then we have the start of a word
                saved_col_number = col_number;
                prevWhitespace = 0;
            } else if (isspace(buffer[ii])) {
                // If the current character is whitespace, then mark prevWhitespace as 1 so future iterations know that the previous character was whitespace
                prevWhitespace = 1;
                continue;
            }

            word[jj] = buffer[ii];
            jj++;
        }
        if (bytesRead == 0) {
            break;
        }
    }
    if (bytesRead == -1) {
        perror("Error reading from file");
        return -1;
    }

    return 0;
}

/**
 * Handle file opening, closing, and calling the appropriate function
 * 
 * @param pathname: The path of the file to open
 * @param func: The function to call
 * @return int: 0 if successful, -1 if an error occurred
*/
int file_handler(const char* pathname, int (*func)()) {
    int fd = open(pathname, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return -1;
    }

    if (func == create_dict && func(fd) == -1) {
        return -1;
    } else if (func(fd, pathname) == -1) {
        return -1;
    }

    if (close(fd) == -1) {
        perror("Error closing file");
        return -1;
    }
    return 0;
}

/**
 * Main function to handle command line arguments and call the appropriate functions
 * 
 * @return int: EXIT_SUCCESS if all words are correct
 *              EXIT_FAILURE if an error occurred or any words are incorrect
*/
int main(int argc, char* argv[]) {
    if (argc <= 2) {
        fprintf(stderr, "Usage: ./spchk <dictionary file> <input file>\n");
        return EXIT_FAILURE;
    }
    
    if (init_trie() == -1) {
        free_trie();
        return EXIT_FAILURE;
    }
    
    for (int ii = 1; ii < argc; ii++) {
        // for each entry, check if valid entry
        // for the second + entries, check that it does not begin with `.` 
        if (ii == 1) {
            if (file_handler(argv[ii], create_dict) == -1) {
                free_trie();
                return EXIT_FAILURE;
            }
        } else {
            int dirname_len = strlen(argv[ii]);
            if (argv[ii][dirname_len - 1] == '/') {
                char directory_name[dirname_len + 1];
                memmove(directory_name, argv[ii], dirname_len + 1);
                directory_name[dirname_len] = '\0';

                DIR* dir = opendir(argv[ii]);
                if (dir == NULL) {
                    perror("Error opening directory");
                    free_trie();
                    return EXIT_FAILURE;
                }
                
                struct dirent *entry;
                while ((entry = readdir(dir)) != NULL) {
                    char* name = entry->d_name;
                    int name_len = strlen(name);
                    int is_txt = strncmp(&name[name_len - 4], ".txt", 4) == 0;
                    
                    if (name[0] == '.' || !is_txt) {
                        continue;
                    }

                    char full_path[dirname_len + name_len + 1];
                    memcpy(full_path, directory_name, dirname_len + 1);
                    strcat(full_path, name);

                    if (file_handler(full_path, check_text) == -1) {
                        free_trie();
                        return EXIT_FAILURE;
                    }
                }              

                if (closedir(dir) == -1) {
                    perror("Error closing directory");
                    free_trie();
                    return EXIT_FAILURE;
                }
            } else {
                if (file_handler(argv[ii], check_text) == -1) {
                    free_trie();
                    return EXIT_FAILURE;
                }
            }
        }
    }
    
    if (SUCCESS) {
        printf("All words are spelled correctly\n");
    }

    free_trie();
    return SUCCESS ? EXIT_SUCCESS : EXIT_FAILURE;
}
