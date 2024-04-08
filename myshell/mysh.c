#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <glob.h>
#include <sys/wait.h>
#include "arraylist.h"
#include "commands.h"
#include "status.h"

#define BUFFER_SIZE 1024

enum { NO_PIPE, PIPE_READ, PIPE_WRITE };

const char* PROMPT      = "mysh> ";
const char* WELCOME_MSG = "Welcome to mysh!\n";
const char* EXIT_MSG    = "Exiting mysh...\n";

typedef struct {
    int fd;
    int pos;
    ssize_t len;
    char buffer[BUFFER_SIZE];
} input_stream;

typedef struct {
    char* program;
    arraylist_t* arguments; 
    arraylist_t* redir_inputs; // Anything succeeding '<'
    arraylist_t* redir_outputs;  // Anything succeeding '>'
} command;


command* command_init(char* program) {
    command* com = malloc(sizeof(command));
    if (com == NULL) {
        perror("Error allocating memory");
        set_exit_status(FAILURE);
        return NULL;
    }
    com->program = malloc(strlen(program) + 1);
    if (com->program == NULL) {
        perror("Error allocating memory");
        free(com);
        set_exit_status(FAILURE);
        return NULL;
    }
    com->arguments = al_init(10);
    if (com->arguments == NULL) {
        free(com);
        free(com->program);
        set_exit_status(FAILURE);
        return NULL;
    }
    com->redir_inputs = al_init(10);    
    if (com->redir_inputs == NULL) {
        free(com);
        free(com->program);
        free(com->arguments);
        set_exit_status(FAILURE);
        return NULL;
    }
    com->redir_outputs = al_init(10);
    if (com->redir_outputs == NULL) {
        free(com);
        free(com->program);
        free(com->arguments);
        free(com->redir_inputs);
        set_exit_status(FAILURE);
        return NULL;
    }

    return com;
}

void command_destroy(command* com) {
    al_destroy(com->redir_outputs);
    al_destroy(com->redir_inputs);
    al_destroy(com->arguments);
    free(com->program);
    free(com);
}

// TODO: fix this function not properly adding args
int command_populate(command* com, arraylist_t* tokens, int start, int end) {
    // NOTE: this only makes shallow copies of the strings; therefore, if any changes are 
    // made to a string in the original tokens arraylist, the same changes will be reflected
    // in its copy in com
    memcpy(com->program, al_get(tokens, start), strlen(al_get(tokens, start)) + 1);
    for (int i = start; i < end; i++) {
        if (strncmp(al_get(tokens, i), "<", 1) == 0) {
            if (i + 1 < end) {
                al_push(com->redir_inputs, al_get(tokens, i + 1));
                i++;
            } else {
                set_exit_status(FAILURE);
                fprintf(stderr, "Error: No input file provided.\n");
                return -1;
            }
        } else if (strncmp(al_get(tokens, i), ">", 1) == 0) {
            if (i + 1 < end) {
                al_push(com->redir_outputs, al_get(tokens, i + 1));
                i++;
            } else {
                set_exit_status(FAILURE);
                fprintf(stderr, "Error: No output file provided.\n");
                return -1;
            }
        } else {
            al_push(com->arguments, al_get(tokens, i));
        }
    }
    al_push(com->arguments, NULL);
    return 0;
}

input_stream* input_stream_init(int fd) {
    input_stream* str = malloc(sizeof(input_stream));
    if (str == NULL) {
        perror("Error allocating memory");
        set_exit_status(FAILURE);
        return NULL;
    }
    str->fd = fd;
    str->pos = 0;
    str->len = 0;

    return str;
}


char* read_line(input_stream *stream) {
    if (stream->fd < 0) return NULL;

    char *line = NULL;
    int line_length = 0;
    int segment_start = stream->pos;

    while (1) {
		if (stream->pos == stream->len) {
		    if (segment_start < stream->pos) {
				int segment_length = stream->pos - segment_start;
				line = realloc(line, line_length + segment_length + 1);
                if (line == NULL) {
                    perror("Error allocating memory");
                    set_exit_status(FAILURE);
                    return NULL;
                }
				memcpy(line + line_length, stream->buffer + segment_start, segment_length);
				line_length = line_length + segment_length;
				line[line_length] = '\0';
		    }

		    stream->len = read(stream->fd, stream->buffer, BUFFER_SIZE);

		    if (stream->len == 0) {
				close(stream->fd);
				stream->fd = -1;
				return line;
		    } else if (stream->len == -1) {
                perror("Error reading from file");
                close(stream->fd);
                stream->fd = -1;
                set_exit_status(FAILURE);
                return NULL;
            }

		    stream->pos = 0;
		    segment_start = 0;
		}

		while (stream->pos < stream->len) {
		    if (stream->buffer[stream->pos] == '\n') {
				int segment_length = stream->pos - segment_start;
				line = realloc(line, line_length + segment_length + 1);
                if (line == NULL) {
                    perror("Error allocating memory");
                    set_exit_status(FAILURE);
                    return NULL;
                }
				memcpy(line + line_length, stream->buffer + segment_start, segment_length);
				line[line_length + segment_length] = '\0';
				stream->pos++;

				return line;
		    }
		    stream->pos++;
		}
    }
    return NULL;
}

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

int wildcard_expansion(arraylist_t* tokens, int pos) {
    // wildcard token
    if (strchr(al_get(tokens, pos), '*') == NULL) return 0;

    glob_t glob_result;
    size_t ret = 0;

    // expand wildcard and add each match to token list
    // GLOB_NOCHECK: if wildcard does not match any files, this returns the original token and is added to the list
    if (glob(al_get(tokens, pos), GLOB_NOCHECK, NULL, &glob_result) == 0) {
        // remove element at pos from arraylist
        al_remove(tokens, pos, NULL);
        for (; ret < glob_result.gl_pathc; ret++) {
            // push at i + j + 1 to maintain proper order
            al_insert(tokens, pos + ret, glob_result.gl_pathv[ret]);
        }
    } else {
        fprintf(stderr, "Error in globbing\n");
        set_exit_status(FAILURE);
        return al_length(tokens);
    }
    globfree(&glob_result);   
    return ret - 1;
}

int handle_built_in_commands(char* program, arraylist_t* args, int stdin_fd) {
    dup2(stdin_fd, STDIN_FILENO);
    if (strcmp(program, "exit") == 0) {
        return exit_shell(args);
    } else if (strcmp(program, "cd") == 0) {
        return cd(args);
    } else if (strcmp(program, "pwd") == 0) {
        return pwd();
    } else if (strcmp(program, "which") == 0) {
        return which(args);
    }

    return 1;
}


void execute_command(command* com, int pipeStatus, int pipefd) {
    int prevStdin = dup(STDIN_FILENO);

    // Pipe before input redirection as redirection has preference
    if (pipeStatus == PIPE_READ) dup2(pipefd, STDIN_FILENO);
    if (al_length(com->redir_inputs) > 0) {
        int in = open(al_get(com->redir_inputs, al_length(com->redir_inputs) - 1), O_RDONLY);
        if (in == -1) {
            perror("Error opening input file");
            set_exit_status(FAILURE);
        }
        dup2(in, STDIN_FILENO);
        close(in);
    }

    // Pipe before output redirection as redirection has preference
    if (pipeStatus == PIPE_WRITE) dup2(pipefd, STDOUT_FILENO);
    if (al_length(com->redir_outputs) > 0) {
        int out;
        for (unsigned i = 0; i < al_length(com->redir_outputs); i++) {
            out = open(al_get(com->redir_outputs, i), O_CREAT | O_WRONLY | O_TRUNC, 0640);
            if (out == -1) {
                perror("Error opening output file");
                set_exit_status(FAILURE);
            }
        }
        dup2(out, STDOUT_FILENO);
        close(out);
    }
    
    // printf("\nExecuting program: %s\n", com->program);
    int builtInStatus = handle_built_in_commands(com->program, com->arguments, prevStdin);
    if (builtInStatus == 1) {
        set_exit_status(SUCCESS);
        if (execv(com->program, com->arguments->data) == -1) {
            perror("Error executing command\n");
            set_exit_status(FAILURE);
        }
    }
}

void parse_and_execute(input_stream* stream) {
    char* line = read_line(stream);
    if (line == NULL || strlen(line) == 0){
        return;
    }
    arraylist_t* tokens = tokenize(line);
    // for (unsigned i = 0; i < al_length(tokens); i++) {
    //     printf("%s\n", al_get(tokens, i));
    // }
    // printf("\n");
    free(line);

    // check for conditionals (then, else)
    if (strcmp(al_get(tokens, 0), "then") == 0) {
        // check if the previous command was successful
        if (get_exit_status() == SUCCESS) {
            al_remove(tokens, 0, NULL);
        } else {
            return;
        }
    } else if (strcmp(al_get(tokens, 0), "else") == 0) {
        // check if the previous command was not successful            
        if (get_exit_status() == FAILURE) {
            al_remove(tokens, 0, NULL);
        } else {
            return;
        }
    } else if (strcmp(al_get(tokens, 0), "exit") == 0) {
        command* com = command_init(al_get(tokens, 0));
        command_populate(com, tokens, 0, al_length(tokens));
        exit_shell(com->arguments);
    }

    // wildcard expansion
    for (unsigned i = 0; i < al_length(tokens); i++) {
        i += wildcard_expansion(tokens, i);
    }

    // find pipeline index. if no pipeline, just create 1 command
    // if pipeline exists, create command for before and after
    int pipelineIndex = -1;
    for (unsigned i = 0; i < al_length(tokens); i++) {
        if (strncmp(al_get(tokens, i), "|", 1) == 0) {
            pipelineIndex = i;
            break;
        }
    }

    if (pipelineIndex == -1) {
        //make child process
        char* progPath = handle_program_path(al_get(tokens, 0));
        if (strlen(progPath) == 0) {
            set_exit_status(FAILURE);
            printf("%s: command not found\n", al_get(tokens, 0));
            return;
        }
        al_set(tokens, 0, progPath);
        free(progPath);

        pid_t child = fork();
        if (child == -1) {
            perror("Error forking");
            set_exit_status(FAILURE);
        } else if (child == 0) {
            // inside child process
            command* com = command_init(al_get(tokens, 0));
            if (command_populate(com, tokens, 0, al_length(tokens)) == -1) {
                command_destroy(com);
                return;
            }
            execute_command(com, NO_PIPE, 0);
            command_destroy(com);
        } else {
            // inside parent process
            int status;
            waitpid(child, &status, 0);
            if (WIFEXITED(status)) {
                // child process exited normally
                int exit_status = WEXITSTATUS(status); // a number between 0 - 255, 0 = success, any other number indicates error
                if (exit_status == 0) {
                    set_exit_status(SUCCESS);
                } else {
                    set_exit_status(FAILURE);
                }
            } else if (WIFSIGNALED(status)) {
                // child process was terminated by a signal (ex, segfault errors, etc.)
                set_exit_status(FAILURE);
            }
        }
    } else {
        int pipefd[2]; // pipefd[0] = read, pipefd[1] = write
        if (pipe(pipefd) == -1) {
            perror("Error creating pipe");
            set_exit_status(FAILURE);
            return;
        }

        char* progPath1 = handle_program_path(al_get(tokens, 0));
        int err = 0; // to keep track of fist command not existing through child
        if (strlen(progPath1) == 0) {
            printf("%s: command not found\n", al_get(tokens, 0));
            dup2(STDOUT_FILENO, pipefd[1]);
            dup2(STDIN_FILENO, pipefd[0]);
            err = -1;
        }
        al_set(tokens, 0, progPath1);
        free(progPath1);

        char* progPath2 = handle_program_path(al_get(tokens, pipelineIndex + 1));
        if (strlen(progPath2) == 0) {
            set_exit_status(FAILURE);
            printf("%s: command not found\n", al_get(tokens, pipelineIndex + 1));
            return;
        }
        al_set(tokens, pipelineIndex + 1, progPath2);
        free(progPath2);

        // 2 child processes, 1 for each command in the pipe
        pid_t child1 = fork();
        if (child1 == -1) {
            perror("Error forking child1");
            set_exit_status(FAILURE);
        } else if (child1 == 0) {
            if (err == 0) {
                command* com1 = command_init(al_get(tokens, 0));
                if (command_populate(com1, tokens, 0, pipelineIndex)) {
                    command_destroy(com1);
                    return;
                }
                execute_command(com1, PIPE_WRITE, pipefd[1]);
                command_destroy(com1);
            }
        } 

        close(pipefd[1]);

        pid_t child2 = fork();
        if (child2 == -1) {
            perror("Error forking child2");
            set_exit_status(FAILURE);
        } else if (child2 == 0) {
            command* com2 = command_init(al_get(tokens, 0));
            if (command_populate(com2, tokens, pipelineIndex + 1, al_length(tokens))) {
                command_destroy(com2);
                return;
            }
            execute_command(com2, PIPE_READ, pipefd[0]);
            command_destroy(com2);
        }
        
        close(pipefd[0]);

        int status1, status2;
        waitpid(child1, &status1, 0);
        waitpid(child2, &status2, 0);

        // TODO: gets exit status of last command
        printf("STATUS %d %d\n", status1, status2);
        if (WIFEXITED(status1) || WIFEXITED(status2)) {
            // child process exited normally
            int exit_status2 = WEXITSTATUS(status2); // a number between 0 - 255, 0 = success, any other number indicates error
            // printf("STATUS %d\n", exit_status2);
            if (exit_status2 == 0) {
                set_exit_status(SUCCESS);
            } else {
                set_exit_status(FAILURE);
            }
        } else {
            // child process was terminated by a signal (ex, segfault errors, etc.)
            set_exit_status(FAILURE);
        }
    }
    al_destroy(tokens);
}


// iterate through all the lines in the file and parse and execute each line
void batch_mode(int fd) {
    input_stream* stream = input_stream_init(fd);
    while (stream != NULL && stream->fd != -1) {
        parse_and_execute(stream);        
    }   
    free(stream);
}

void interactive_mode() {
    printf("%s", WELCOME_MSG);
    input_stream* stream = input_stream_init(STDIN_FILENO);
    while (stream != NULL && get_exit_flag() == CONTINUE) {
        printf("%s", PROMPT);
        fflush(stdout);
        parse_and_execute(stream);
    }
    free(stream);
    printf("%s", EXIT_MSG);
}

int main(int argc, char** argv) {
    int systemInput = isatty(STDIN_FILENO);

    // argc == 1 and systemInput == 1 means interactive mode
    // argc == 1 and systemInput == 0 means batch mode with no file provided (piping)
    // argc == 2 means batch mode with file provided
    if (argc == 1) {
        if (systemInput == 1) {
            interactive_mode();
        } else {
            // batch mode with no file provided
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