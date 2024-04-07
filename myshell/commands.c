#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "commands.h"
#include "status.h"

#define BUFFER_SIZE 1024

void cd(arraylist_t* tokens) {
    if (al_length(tokens) == 0) {
        fprintf(stderr, "Error: cd requires an argument\n");
        set_exit_status(FAILURE);
        return;
    } else if (al_length(tokens) > 1) {
        fprintf(stderr, "Error: cd only accepts one argument\n");
        set_exit_status(FAILURE);
        return;
    }
    char* path = al_get(tokens, 0);
    if (chdir(path) == -1) {
        perror("Error using cd");
        set_exit_status(FAILURE);
    }
    set_exit_status(SUCCESS);
}

void pwd() {
    char* currentWorkingDirectory = malloc(BUFFER_SIZE);
    if (currentWorkingDirectory == NULL) {
        perror("Error using pwd");
        set_exit_status(FAILURE);
        return;
    }
    size_t buffer = BUFFER_SIZE;
    while (getcwd(currentWorkingDirectory, buffer) == NULL) {
        if (errno != ERANGE) {
            perror("Error using pwd");
            set_exit_status(FAILURE);
            return;
        } else {
            buffer *= 2;
            currentWorkingDirectory = realloc(currentWorkingDirectory, buffer);
            if (currentWorkingDirectory == NULL) {
                perror("Error using pwd");
                set_exit_status(FAILURE);
                return;
            }
        }
    }
    printf("%s\n", currentWorkingDirectory);
    free(currentWorkingDirectory);
    set_exit_status(SUCCESS);
}

char* handle_program_path(char* program) {
    const char* directories[] = {"/usr/local/bin", "/usr/bin", "/bin"};
    int numDirectories = sizeof(directories) / sizeof(directories[0]);

    size_t pathLength = strlen(program) + 1 + 1 + 14; // 1 for null terminator, 1 for '/', 14 for "/usr/local/bin"
    char* path = malloc(pathLength); 

    // check if the program contains a slash, ex ./myprogram
    if (strchr(program, '/') != NULL) {
        strncpy(path, program, strlen(program) + 1);
        return path;
    }

    // handle case when there is no slash, ex. myprogram. in this case, we look through directories[] to find the program
    for (int i = 0; i < numDirectories; i++) {
        sprintf(path, "%s/%s", directories[i], program);
        if (access(path, X_OK) == 0) {
            return path;
        }
    }
    // // program wasn't found, so leave path empty
    // memset(path, 0, pathLength);
    // return path;
    strncpy(path, program, strlen(program) + 1);
    return program;
}

/**
 * This function is used to find the location of a program
 * 
 * NOTE: does not print anything upon error status
*/
void which(arraylist_t* tokens) {
    if (al_length(tokens) == 0) {
        set_exit_status(FAILURE);
        return;
    } else if (al_length(tokens) > 1) {
        set_exit_status(FAILURE);
        return;
    }

    char* program = al_get(tokens, 0);
    char* path = handle_program_path(program);
    if (strlen(program) == strlen(path) || strlen(path) == 0) {
        free(path);
        set_exit_status(FAILURE);
        return;
    } 
    printf("%s\n", path);
    free(path);
    set_exit_status(SUCCESS);
}

void exit_shell(arraylist_t* tokens) {
    if (al_length(tokens) > 0) {
        for (unsigned i = 0; i < al_length(tokens); i++) {
            printf("%s ", al_get(tokens, i));
        }
    }
    set_exit_status(SUCCESS);
    // exit(get_exit_status());
}
