#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
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
arraylist_t* tokenize(char* line, char* tokens[]) {
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

void batch_mode(int fd) {
    
    

}

void interactive_mode() {
    printf(WELCOME_MSG);
    while (1) {
        printf(PROMPT);
        
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