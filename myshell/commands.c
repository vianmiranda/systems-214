#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "commands.h"
#include "status.h"

#define BUFFER_SIZE 1024

void cd(arraylist_t* tokens) {
    if (al_length(tokens) < 3) { 
        // cd is defualt arg1, NULL is default arg2
        printf("Error: cd requires an argument\n");
        set_exit_status(FAILURE);
        return;
    } else if (al_length(tokens) > 3) {
        // cd is defualt arg1, NULL is default argn
        printf("Error: cd only accepts one argument\n");
        set_exit_status(FAILURE);
        return;
    }
    char* path = al_get(tokens, 1);
    if (chdir(path) == -1) {
        perror("Error using cd");
        set_exit_status(FAILURE);
        return;
    }
    set_exit_status(SUCCESS);
    return;
}

void pwd() {
    char* currentWorkingDirectory = malloc(BUFFER_SIZE);
    if (currentWorkingDirectory == NULL) {
        perror("Error using pwd");
        exit(FAILURE);
    }
    size_t buffer = BUFFER_SIZE;
    while (getcwd(currentWorkingDirectory, buffer) == NULL) {
        if (errno != ERANGE) {
            perror("Error using pwd");
            exit(FAILURE);
        } else {
            buffer *= 2;
            currentWorkingDirectory = realloc(currentWorkingDirectory, buffer);
            if (currentWorkingDirectory == NULL) {
                perror("Error using pwd");
                exit(FAILURE);
            }
        }
    }
    printf("%s\n", currentWorkingDirectory);
    free(currentWorkingDirectory);
    exit(SUCCESS);
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

    // check if the program is a built-in command
    const char* builtInCommands[] = {"cd", "pwd", "which", "exit"};
    for (int i = 0; i < (int) (sizeof(builtInCommands) / sizeof(builtInCommands[0])); i++) {
        if (strncmp(program, builtInCommands[i], strlen(builtInCommands[i])) == 0) {
            strncpy(path, program, strlen(program) + 1);
            return path;
        }
    }

    // handle case when there is no slash, ex. myprogram. in this case, we look through directories[] to find the program
    for (int i = 0; i < numDirectories; i++) {
        sprintf(path, "%s/%s", directories[i], program);
        if (access(path, X_OK) == 0) {
            return path;
        }
    }
    // // program wasn't found, so leave path empty
    memset(path, 0, pathLength);
    return path;
}

/**
 * This function is used to find the location of a program
 * 
 * NOTE: does not print anything upon error status
*/
void which(arraylist_t* tokens) {
    if (al_length(tokens) < 3) {
        // which is defualt arg1, NULL is default arg2
        exit(FAILURE);
    } else if (al_length(tokens) > 3) {
        // which is defualt arg1, NULL is default argn
        exit(FAILURE);
    }

    char* program = al_get(tokens, 1);
    char* path = handle_program_path(program);
    if (strlen(program) == strlen(path) || strlen(path) == 0) {
        free(path);
        exit(FAILURE);
    } 
    printf("%s\n", path);
    free(path);
    exit(SUCCESS);
}

void exit_shell(arraylist_t* tokens) {
    if (al_length(tokens) > 2) {
        for (unsigned i = 1; i < al_length(tokens) - 1; i++) {
            printf("%s ", al_get(tokens, i));
        }
        printf("\n");
        printf("pid: %d\n\n", getpid());
    }
    kill(getpid(), SIGTERM);
    exit(SUCCESS);
}
