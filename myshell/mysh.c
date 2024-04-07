#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <glob.h>
#include "arraylist.h"
#include "commands.h"
#include "status.h"

#define BUFFER_SIZE 1024

const char* PROMPT      = "mysh> ";
const char* WELCOME_MSG = "Welcome to mysh!\n";
const char* EXIT_MSG    = "Exiting mysh...\n";

// tokenize the line into tokens
arraylist_t* tokenize(char* line) {
    char* token = strtok(line, " ");
    arraylist_t* list = al_init(10);

    while (token != NULL) {
        int start = 0, len = strlen(token);
        for (int i = 0; i < len && len != 1; i++) {
            if (token[i] == '<' || token[i] == '>' || token[i] == '|') {
                char save[2] = {token[i], '\0'};
                if (start < i) {
                    token[i] = '\0';
                    al_push(list, &token[start]);
                }
                al_push(list, save);
                start = i + 1;
            }
        }
        if (start < len) al_push(list, &token[start]);
        token = strtok(NULL, " ");
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

void handle_built_in_commands(arraylist_t* tokens) {
    if (strcmp(al_get(tokens, 0), "exit") == 0) {
        exit_shell(tokens);
    } else if (strcmp(al_get(tokens, 0), "cd") == 0) {
        cd(tokens);
    } else if (strcmp(al_get(tokens, 0), "pwd") == 0) {
        pwd();
    } else if (strcmp(al_get(tokens, 0), "which") == 0) {
        which(tokens);
    }
}

void handle_wildcard(char* token, arraylist_t* tokens, int pos) {
    // wildcard token
    glob_t* glob_result;

    // expand wildcard and add each match to token list
    // GLOB_NOCHECK: if wildcard does not match any files, this returns the original token and is added to the list
    if (glob(token, GLOB_NOCHECK, NULL, glob_result) == 0) {
        // remove element at pos from arraylist
        al_remove(tokens, pos, NULL);
        for (int j = 0; j < (int) glob_result->gl_pathc; j++) {
            // push at i + j + 1 to maintain proper order
            al_put(tokens, pos + j, glob_result->gl_pathv[j]);
        }
    }
    globfree(glob_result);   
}



void execute_command(char* command, int redirect_input, int redirect_output, char* inputFile, char* outputFile) {
    
    // fork
    pid_t pid = fork();
    if (pid == -1) {
        perror("Error forking");
        set_exit_status(FAILURE);
    } else if (pid == 0) {
        // we are inside child process
        if (redirect_input) {

        }

        if (redirect_output) {

        }

        // execute command

    } else {
        // parent process
        int status;
        wait(&status);
        // handle status
    }
}



void batch_mode(int fd) {
}



void interactive_mode() {

    // to keep track of the commands, especially for pipelines
    arraylist_t* commands;
    int commandCount = 0;

    // input and output file
    char* inputFile = NULL, outputFile = NULL;

    // to keep track of the status of redirection operators
    int redirect_input = 0, redirect_output = 0;

    // pipe status
    int pipeStatus = 0;


    printf("%s\n", WELCOME_MSG);

    while (1) {
        printf("%s\n", PROMPT);
        char* line = read_line(STDIN_FILENO);
        arraylist_t* tokens = tokenize(line);

        // check for conditionals (then, else)
        if (strcmp(al_get(tokens, 0), "then") == 0) {
            // check if the previous command was successful

        } else if (strcmp(al_get(tokens, 0), "else") == 0) {
            // check if the previous command was not successful
        }

        // check for built-in commands
        handle_built_in_commands(tokens);

        // check for redirection operators
        for (int i = 0; i < al_length(tokens); i++) {
            if (strcmp(al_get(tokens, i), "<") == 0) {
                redirect_input = 1;
                // get input file
                
            } else if (strcmp(al_get(tokens, i), ">") == 0) {
                redirect_output = 1;
                // get output file
            } else if (strcmp(al_get(tokens, i), "|") == 0) {
                // handle pipe
                pipeStatus = 1;


            }
        }

        // execute command in pipeline
        for (int i = 0; i <= commandCount; i++) {
            execute_command(al_get(commands, i), redirect_input, redirect_output, inputFile, outputFile);
        }

    }

    printf("%s\n", EXIT_MSG);
}



int main(int argc, char** argv) {
    // char line[] = "ls -l | wc -l";
    // printf("%s\n", line);
    // arraylist_t* tokens = tokenize(line);

    // // print the tokens
    // for (int i = 0; i < al_length(tokens); i++) {
    //     printf("%s\n", al_get(tokens, i));
    // }

    // al_destroy(tokens);
    
    int systemInput = isatty(STDIN_FILENO);

    // argc == 1 and systemInput == 1 means interactive mode
    // argc == 1 and systemInput == 0 means batch mode with no file provided (piping)
    // argc == 2 means batch mode with file provided
    if (argc == 1) {
        if (systemInput == 1) {
            interactive_mode();
        } else {
            batch_mode(STDIN_FILENO);
        }
    } else if (argc == 2) {
        int fd = open(argv[1], O_RDONLY);
        batch_mode(fd);
    } else {
        fprintf(stderr, "Error: Too many arguments; Usage: mysh <batchfile>\n");
        return -1;
    }

    return 0;
}