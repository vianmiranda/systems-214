#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "commands.h"


static int exitStatus = SUCCESS;

int get_exit_status() {
    return exitStatus;
}

void set_exit_status(int status) {
    exitStatus = status;
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