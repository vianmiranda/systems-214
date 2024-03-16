#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "triedict.h"

#define BUFFER_SIZE 128

int create_dict(int fd) {
    // read from file and create trie
    char buffer[BUFFER_SIZE];
    if (buffer == NULL) {
        perror("Error allocating memory");
        return -1;
    }

    if (init_trie() == -1) {
        return -1;
    }
    
    char word[BUFFER_SIZE];
    ssize_t jj = 0, bytesRead = 0;
    while ((bytesRead = read(fd, buffer, BUFFER_SIZE)) > 0) {
        for (ssize_t ii = 0; ii < bytesRead; ii++) {
            if (buffer[ii] == '\n') {
                word[jj] = '\0';
                jj = 0;
                if (add_word_to_trie(word) == -1) {
                    return -1;
                }
                memset(word, 0, BUFFER_SIZE);
                continue;
            }
            word[jj] = buffer[ii];
            jj++;
        }
    }
    if (bytesRead == -1) {
        perror("Error reading from file");
        return -1;
    }

    return 0;
}

char** clean_text(char* word) {
    // text -> {dict varitions}
    // "apple" -> {"apple"}
    // "Apple" -> {"apple", "Apple"}
    // "APPLE" -> {"apple", "Apple", "APPLE"}
    // "MacDonald" -> {"MacDonald"}
    // "HeLlO" -> {"HeLlO"}

    const int exactMatch = 1; // If none of these below are true, then the word must match exactly. Always true by default
    int allLowercase = 0; // strictest case
    int properCase = 0; // allLowercase will always be true if this is true
    int allUppercase = 0; // allLowercase & properCase will always be true if this is true

    int trailingPunctuation;
    for (trailingPunctuation = strlen(word) - 1; trailingPunctuation >= 0; trailingPunctuation--) {
        if ((word[trailingPunctuation] >= 'A' && word[trailingPunctuation] <= 'Z') || (word[trailingPunctuation] >= 'a' && word[trailingPunctuation] <= 'z')) {
            break;
        }
    }

    int leadingPunctuation;
    for (leadingPunctuation = 0; leadingPunctuation < strlen(word); leadingPunctuation++) {
        if ((word[leadingPunctuation] >= 'A' && word[leadingPunctuation] <= 'Z') || (word[leadingPunctuation] >= 'a' && word[leadingPunctuation] <= 'z')) {
            break;
        }
    }

    int wordLen = trailingPunctuation - leadingPunctuation + 1;
    if (wordLen == 0) {
        return malloc(0);
    }

    int numUppercase = 0;
    int ignorePunctuation = wordLen;
    for (int ii = leadingPunctuation; ii <= trailingPunctuation; ii++) {
        if (word[ii] >= 'A' && word[ii] <= 'Z') {
            numUppercase += 1;
            if (ii == 0) {
                properCase = 1;
            } else {
                properCase = 0;
            }
        } else if (!(word[ii] >= 'a' && word[ii] <= 'z')) {
            ignorePunctuation -= 1;
        }
    }
    

    if (numUppercase == ignorePunctuation) {
        allLowercase = 1;
        properCase = 1;
        allUppercase = 1;
    }
    if (numUppercase == 0) {
        allLowercase = 1;
    }

    char* parsed = word + leadingPunctuation;
    parsed[wordLen] = '\0';
    wordLen++;

    int numVariations = exactMatch + allLowercase + properCase + allUppercase;

    char** res = malloc(numVariations * (wordLen * sizeof(char))); // TODO: free this
    if (res == NULL) {
        perror("Error allocating memory");
        return NULL;
    }
    
    if (exactMatch) {
        res[0] = strncpy(res[0], parsed, wordLen);
    } else if (allLowercase) {
        for (int ii = 0; ii < wordLen; ii++) {
            if (parsed[ii] >= 'A' && parsed[ii] <= 'Z') {
                parsed[ii] = parsed[ii] + 32;
            }
        }
        res[1] = strncpy(res[1], parsed, wordLen);
    } else if (properCase) {
        parsed[0] = parsed[0] - 32;
        res[2] = strncpy(res[2], parsed, wordLen);
    } else if (allUppercase) {
        for (int ii = 0; ii < wordLen; ii++) {
            if (parsed[ii] >= 'a' && parsed[ii] <= 'z') {
                parsed[ii] = parsed[ii] - 32;
            }
        }
        res[3] = strncpy(res[3], parsed, wordLen);
    }
    
    return res;
}

int check_text(int fd) {
    // read from file and check against trie1
    char buffer[BUFFER_SIZE];
    if (buffer == NULL) {
        perror("Error allocating memory");
        return -1;
    }
    
    int line_number = 0, col_number = 0;
    char word[BUFFER_SIZE];
    ssize_t jj = 0, bytesRead = 0;
    while ((bytesRead = read(fd, buffer, BUFFER_SIZE)) > 0) {
        for (ssize_t ii = 0; ii < bytesRead; ii++) {
            if (buffer[ii] == ' ') { // TODO: add more delimiters (whitespace and hyphens)

            }
            word[jj] = buffer[ii];
            jj++;
        }
    }
    if (bytesRead == -1) {
        perror("Error reading from file");
        return -1;
    }

    return 0;
}

int file_handler(const char* pathname, int (*func)()) {
    int fd = open(pathname, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return -1;
    }
    
    if (func(fd) == -1) {
        return -1;
    }

    if (close(fd) == -1) {
        perror("Error closing file");
        return -1;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc <= 2) {
        fprintf(stderr, "Usage: ./spchk <dictionary file> <input file>\n");
        return EXIT_FAILURE;
    }
    
    for (int ii = 1; ii < argc; ii++) {
        // for each entry, check if valid entry
        // for the second + entries, check that it does not begin with `.` 
        if (ii == 1) {
            if (file_handler(argv[ii], create_dict) == -1) {
                return EXIT_FAILURE;
            }
        } else {
            int dirname_len = strlen(argv[ii]);
            if (argv[ii][dirname_len - 1] == '/') {
                DIR* dir = opendir(argv[ii]);
                if (dir == NULL) {
                    perror("Error opening directory");
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

                    if (file_handler(name, check_text) == -1) {
                        return EXIT_FAILURE;
                    }
                }              

                if (closedir(dir) == -1) {
                    perror("Error closing directory");
                    return EXIT_FAILURE;
                }
            } else {
                if (file_handler(argv[ii], check_text) == -1) {
                    return EXIT_FAILURE;
                }
            }
        }
    }
    
    return EXIT_SUCCESS;
}


