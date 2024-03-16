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
    ssize_t bytesRead = 0;
    char* buffer = malloc(BUFFER_SIZE);
    if (buffer == NULL) {
        perror("Error allocating memory");
        return -1;
    }

    if (init_trie() == -1) {
        free(buffer);
        return -1;
    }
    
    char* word = malloc(BUFFER_SIZE);
    ssize_t jj = 0;
    while ((bytesRead = read(fd, buffer, BUFFER_SIZE)) > 0) {
        for (ssize_t ii = 0; ii < bytesRead; ii++) {
            if (buffer[ii] == '\n') {
                word[jj] = '\0';
                jj = 0;
                if (add_word_to_trie(word) == -1) {
                    free(word);
                    free(buffer);
                    return -1;
                }
                memset(word, 0, BUFFER_SIZE);
                continue;
            }
            word[jj] = buffer[ii];
            jj++;
        }
    }
    free(word);
    free(buffer);
    if (bytesRead == -1) {
        perror("Error reading from file");
        return -1;
    }

    return 0;
}

int check_text(int fd) {
    // read from file and check against trie
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
        printf("Usage: ./spchk <dictionary file> <input file>\n");
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


