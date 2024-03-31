#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

// tokenize the line into tokens
int tokenize(char* line, char* tokens[]) {
    int num_tokens = 0;
    char* token = strtok(line, " ");
    while (token != NULL) {
        tokens[num_tokens] = token;
        num_tokens++;
        token = strtok(NULL, " ");
    }
    return num_tokens;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);

    if (fd == -1) {
        perror("Cannot open file");
        return 1;
    }

    //using isatty to determine whether to run in interactive or batch mode
    int is_interactive = isatty(fd); //returns 1 if fd is connected to terminal (interactive), 0 otherwise (batch)

    // write input loop


}