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

cleanText* create_variations(char* word) {
    int exactMatch = 1; // If none of these below are true, then the word must match exactly. Always true by default
    int allLowercase = 0; // strictest case
    int properCase = 0; // allLowercase will always be true if this is true
    int allUppercase = 0; // allLowercase & properCase will always be true if this is true

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
        exactMatch = 0;
        allLowercase = 1;
        properCase = 1;
        allUppercase = 1;
    }
    if (numUppercase > 0) {
        allUppercase = 1;
    } 
    if (numUppercase == ignorePunctuation) {
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

    char parsed[wordLen];
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
        res[iterator] = strncpy(res[iterator], parsed, wordLen);
        iterator++;
    }
    if (allLowercase) {
        for (int ii = 0; ii < wordLen; ii++) {
            if (isupper(parsed[ii])) {
                parsed[ii] = parsed[ii] + 32;
            }
        }
        res[iterator] = strncpy(res[iterator], parsed, wordLen);
        iterator++;
    } if (properCase) {
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

cleanText* clean_text(char* word) {
    // text -> {dict varitions}
    // "apple" -> {"apple"}
    // "Apple" -> {"apple", "Apple"}
    // "APPLE" -> {"apple", "Apple", "APPLE"}
    // "MacDonald" -> {"MacDonald"}
    // "HeLlO" -> {"HeLlO"}

    // int exactMatch = 1; // If none of these below are true, then the word must match exactly. Always true by default
    // int allLowercase = 0; // strictest case
    // int properCase = 0; // allLowercase will always be true if this is true
    // int allUppercase = 0; // allLowercase & properCase will always be true if this is true

    int trailingPunctuation;
    for (trailingPunctuation = strlen(word) - 1; trailingPunctuation >= 0; trailingPunctuation--) {
        if (isalpha(word[trailingPunctuation])) {
            break;
        }
    }

    int leadingPunctuation;
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

    // int wordLen = trailingPunctuation - leadingPunctuation + 1;
    // int numUppercase = 0;
    // int ignorePunctuation = wordLen;
    // for (int ii = leadingPunctuation; ii <= trailingPunctuation; ii++) {
    //     if (isupper(word[ii])) {
    //         numUppercase += 1;
    //         if (ii == leadingPunctuation) {
    //             properCase = 1;
    //         } else {
    //             properCase = 0;
    //         }
    //     } else if (!isalpha(word[ii])) {
    //         ignorePunctuation -= 1;
    //     }
    // }
    
    // if (numUppercase == ignorePunctuation) {
    //     allLowercase = 1;
    //     properCase = 1;
    //     allUppercase = 1;
    // }
    // if (numUppercase == 0 || properCase == 1) {
    //     allLowercase = 1;
    // }

    // cleanText* clean = malloc(sizeof(cleanText));
    // if (clean == NULL) {
    //     perror("Error allocating memory");
    //     return NULL;
    // }
    // if (wordLen <= 0) {
    //     clean->numVariations = 0;
    //     clean->variations = NULL;
    //     return clean;
    // }

    // char parsed[wordLen];
    // strncpy(parsed, word + leadingPunctuation, wordLen);
    // parsed[wordLen] = '\0';
    // wordLen++;

    // int numVariations = exactMatch + allLowercase + properCase + allUppercase;
    // clean->numVariations = numVariations;
    // // printf("%d %d %d %d\n", exactMatch, allLowercase, properCase, allUppercase);
    // char** res = malloc(numVariations * sizeof(char*));
    // if (res == NULL) {
    //     perror("Error allocating memory");
    //     free(clean);
    //     return NULL;
    // }
    // for (int ii = 0; ii < numVariations; ii++) {
    //     res[ii] = malloc(wordLen * sizeof(char));
    //     if (res[ii] == NULL) {
    //         perror("Error allocating memory");
    //         for (int jj = 0; jj < ii; jj++) {
    //             free(res[jj]);
    //         }
    //         free(res);
    //         free(clean);
    //         return NULL;
    //     }
    // }

    // if (exactMatch) {
    //     res[0] = strncpy(res[0], parsed, wordLen);

    //     if (allLowercase) {
    //         for (int ii = 0; ii < wordLen; ii++) {
    //             if (isupper(parsed[ii])) {
    //                 parsed[ii] = parsed[ii] + 32;
    //             }
    //         }
    //         res[1] = strncpy(res[1], parsed, wordLen);

    //         if (properCase) {
    //             parsed[0] = parsed[0] - 32;
    //             res[2] = strncpy(res[2], parsed, wordLen);

    //             if (allUppercase) {
    //                 for (int ii = 0; ii < wordLen; ii++) {
    //                     if (islower(parsed[ii])) {
    //                         parsed[ii] = parsed[ii] - 32;
    //                     }
    //                 }
    //                 res[3] = strncpy(res[3], parsed, wordLen);
    //             }
    //         }
    //     }
    // }
    // clean->variations = res;

    // return clean;
}


// Check each word in the hyphenated word. If the word is not in the dictionary, set SUCCESS = 0 and return -1
// int handle_hyphenated_word(char *word) {
//     char *token = strtok(word, "-");
//     while (token != NULL) {
//         cleanText *cleanWords = clean_text(token);
//         if (cleanWords == NULL) {
//             fprintf(stderr, "Error cleaning hyphenated word: %s\n", word);
//             return -1;
//         }
//         for (int kk = 0; kk < cleanWords->numVariations; kk++) {
//             if (check_word_in_trie(cleanWords->variations[kk]) == 1) {
//                 free(cleanWords->variations[kk]);
//                 free(cleanWords->variations);
//                 free(cleanWords);
//                 return 0;
//             }
//             free(cleanWords->variations[kk]);
//         }
//         free(cleanWords->variations);
//         free(cleanWords);
//         token = strtok(NULL, "-");
//     }
//     // If we reach this point, then none of the variations is correct
//     SUCCESS = 0;
//     return 0;
// }


int check_text(int fd, char* file_name) {
    // read from file and check against trie1
    char buffer[BUFFER_SIZE];
    if (buffer == NULL) {
        perror("Error allocating memory");
        return -1;
    }

    int prevWhitespace = 1;
    int line_number = 1, col_number = 0, saved_col_number;
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

            if (!prevWhitespace && (isspace(buffer[ii]) || (bytesRead == 0 && jj != 0))) { 
                // If the previous character was not whitespace and the current character is whitespace, then we have a word
                word[jj] = '\0';
                jj = 0;
                prevWhitespace = 1;

                // Check if the word contains a hyphen
                // if (word[0] == '-' && jj > 1) {
                //     if (handle_hyphenated_word(word) == -1) {
                //         return -1;
                //     }
                //     continue;
                // }


                cleanText* cleanWord = clean_text(word);
                if (cleanWord == NULL) {
                    return -1;
                } else if (cleanWord->numVariations == 0) {
                    free(cleanWord);
                    memset(word, 0, BUFFER_SIZE);
                    continue;
                }

                if (check_word_in_trie(cleanWord->variations[0]) == 0) {
                    SUCCESS = 0;
                    fprintf(stderr, "%s (%d, %d): %s\n", file_name, (line_number - (buffer[ii] == '\n' ? 1 : 0)), saved_col_number, word);
                }
                free(cleanWord);

                // cleanText* cleanWords = clean_text(word);
                // if (cleanWords == NULL) {
                //     memset(word, 0, BUFFER_SIZE);
                //     return -1;
                // } else if (cleanWords->numVariations == 0) {
                //     free(cleanWords);
                //     memset(word, 0, BUFFER_SIZE);
                //     continue;
                // }

                // int correct = 0;
                // for (int kk = 0; kk < cleanWords->numVariations; kk++) {
                //     // printf("%s\n", cleanWords->variations[kk]);
                //     if (!correct && check_word_in_trie(cleanWords->variations[kk]) == 1) {
                //         correct = 1;
                //     }
                //     free(cleanWords->variations[kk]);
                // }
                // free(cleanWords->variations);
                // free(cleanWords);

                // if (!correct) {
                //     SUCCESS = 0;
                //     fprintf(stderr, "%s (%d, %d): %s\n", file_name, (line_number - (buffer[ii] == '\n' ? 1 : 0)), saved_col_number, word);
                // }

                memset(word, 0, BUFFER_SIZE);
                continue;
            } else if (prevWhitespace && !isspace(buffer[ii])) {
                // If the previous character was whitespace and the current character is not whitespace, then we have the start of a word
                saved_col_number = col_number;
                prevWhitespace = 0;
            } else if (isspace(buffer[ii])) {
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
