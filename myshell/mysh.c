#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <glob.h>
#include "arraylist.h"

#define BUFFER_SIZE 1024

const char* PROMPT      = "mysh> ";
const char* WELCOME_MSG = "Welcome to mysh!\n";
const char* EXIT_MSG    = "Exiting mysh...\n";

enum {
    SUCCESS,
    FAILURE
};

int exitStatus = SUCCESS;

// tokenize the line into tokens
arraylist_t* tokenize(char* line) {
    char* token = strtok(line, " \t");
    arraylist_t* list;
    al_init(list, 10);

    while (token != NULL) {
        int start = 0;
        for (int i = 0; i < strlen(token); i++) {
            if (token[i] == '<' || token[i] == '>' || token[i] == '|') {
                char save = token[i];
                token[i] = '\0';
                al_push(list, &token[start]);
                al_push(list, save);
                start = i + 1;
            }
        }
        al_push(list, &token[start]);
        token = strtok(NULL, " \t");
    }
    return list;
}

char* read_line(int fd) {
    char* line = malloc(0);
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;
    while ((bytesRead = read(fd, buffer, BUFFER_SIZE)) > 0) {
        char* newlineChar = strchr(buffer, '\n');
        if (newlineChar != NULL) {
            *newlineChar = '\0';
            line = realloc(line, strlen(line) + strlen(buffer) + 1);
            strncat(line, buffer, strlen(buffer));
            break;
        } else {
            line = realloc(line, strlen(line) + BUFFER_SIZE + 1);
            strncat(line, buffer, BUFFER_SIZE);
            line[strlen(line) + BUFFER_SIZE] = '\0';
        }
    }
    return line;
}


void cd(char* path) {
    if (chdir(path) == -1) {
        perror("Error using cd");
        exitStatus = FAILURE;
    }
}

void pwd() {
    char currentWorkingDirectory[1024];
    if (getcwd(currentWorkingDirectory, sizeof(currentWorkingDirectory)) == NULL) {
        perror("Error using pwd");
        exitStatus = FAILURE;
    } else {
        printf("%s\n", currentWorkingDirectory);
    }
}


void which(char* program) {
    // PATH is a colon-separated list of directories
    char* path = getenv("PATH");

    // break the list by colon to iterate through each directory
    char* token = strtok(path, ":");
    while (token != NULL) {
        char* fullPath = malloc(strlen(token) + strlen(program) + 2); // we add 2 for the '/' and null terminator

        // combine the directory and program name
        strcpy(fullPath, token);
        strcat(fullPath, "/");
        strcat(fullPath, program);
        
        // check if file exists. if it does, print the path
        if (access(fullPath, F_OK) == 0) {
            printf("%s\n", fullPath);
            return;
        }

        // reset token and move to next directory
        token = strtok(NULL, ":");
        free(fullPath);
    }
    fprintf(stderr, "Error: %s not found\n", program);
    exitStatus = FAILURE;
}

void batch_mode(int fd) {
}


void interactive_mode() {
    printf(WELCOME_MSG);

    while (1) {

        // print prompt
        printf(PROMPT);

        // read line
        char* line = read_line(STDIN_FILENO);

        // tokenize line 
        arraylist_t* tokens = tokenize(line);

        // wildcard expansion using glob. if there is a match, add it to token list
        for (int i = 0; i < al_length(tokens); i++) {
            char* token = al_get(tokens, i);
            if (strchr(token, '*') != NULL) {
                // wildcard token
                glob_t glob_result;
                // initialize glob_result
                memset(&glob_result, 0, sizeof(glob_result));

                // expand wildcard and add each match to token list
                // GLOB_NOCHECK: if wildcard does not match any files, this returns the original token and is added to the list
                if (glob(token, GLOB_NOCHECK, NULL, &glob_result) == 0) {
                    for (int j = 0; i < glob_result.gl_pathc; j++) {
                        // push at i + j + 1 to maintain proper order
                        al_push_at_pos(tokens, i + j + 1, glob_result.gl_pathv[j]);
                    }
                }


            }
        }





        
    }
    printf(EXIT_MSG);
}

int main(int argc, char** argv) {
    int systemInput = isatty(STDIN_FILENO);
    if (argc == 1 && systemInput == 1) {
        // interactive
        interactive_mode();
    } else if (argc == 2) {
        // batch
        int fd = systemInput == 1 ? open(argv[1], O_RDONLY) : STDIN_FILENO;
        batch_mode(fd);
    } else {
        fprintf(stderr, "Error: Too many arguments; Usage: mysh <batchfile>\n");
        return -1;
    }

    return 0;
}