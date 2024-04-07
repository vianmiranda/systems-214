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
        return NULL;
    }
    com->program = malloc(strlen(program) + 1);
    if (com->program == NULL) {
        perror("Error allocating memory");
        free(com);
        return NULL;
    }
    com->arguments = al_init(10);
    if (com->arguments == NULL) {
        free(com);
        free(com->program);
        return NULL;
    }
    com->redir_inputs = al_init(10);    
    if (com->redir_inputs == NULL) {
        free(com);
        free(com->program);
        free(com->arguments);
        return NULL;
    }
    com->redir_outputs = al_init(10);
    if (com->redir_outputs == NULL) {
        free(com);
        free(com->program);
        free(com->arguments);
        free(com->redir_inputs);
        return NULL;
    }

    return com;
}

void command_destroy(command* com) {
    free(com->redir_outputs);
    free(com->redir_inputs);
    free(com->arguments);
    free(com->program);
    free(com);
}

void command_populate(command* com, arraylist_t* tokens, int start, int end) {
    // NOTE: this only makes shallow copies of the strings; therefore, if any changes are 
    // made to a string in the original tokens arraylist, the same changes will be reflected
    // in its copy in com
    com->program = al_get(tokens, 0);
    for (int i = 1; i < end; i++) {
        if (strncmp(al_get(tokens, i), "<", 1) == 0) {
            if (i + 1 < end) {
                al_push(com->redir_inputs, al_get(tokens, i + 1));
                i++;
            } else {
                set_exit_status(FAILURE);
                fprintf(stderr, "No input file provided.");
                return;
            }
        } else if (strncmp(al_get(tokens, i), ">", 1) == 0) {
            if (i + 1 < end) {
                al_push(com->redir_outputs, al_get(tokens, i + 1));
                i++;
            } else {
                set_exit_status(FAILURE);
                fprintf(stderr, "No output file provided.");
                return;
            }
        } else {
            al_push(com->arguments, al_get(tokens, i));
        }
    }
}

input_stream* input_stream_init(int fd) {
    input_stream* str = malloc(sizeof(input_stream));
    str->fd = fd;
    str->pos = 0;
    str->len = 0;
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
				memcpy(line + line_length, stream->buffer + segment_start, segment_length);
				line_length = line_length + segment_length;
				line[line_length] = '\0';
		    }

		    stream->len = read(stream->fd, stream->buffer, BUFFER_SIZE);

		    if (stream->len < 1) {
				close(stream->fd);
				stream->fd = -1;
				return line;
		    }

		    stream->pos = 0;
		    segment_start = 0;
		}

		while (stream->pos < stream->len) {
		    if (stream->buffer[stream->pos] == '\n') {
				int segment_length = stream->pos - segment_start;
				line = realloc(line, line_length + segment_length + 1);
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

void wildcard_expansion(char* token, arraylist_t* tokens, int pos) {
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

int handle_built_in_commands(arraylist_t* tokens) {
    if (strcmp(al_get(tokens, 0), "exit") == 0) {
        exit_shell(tokens);
        return 1;
    } else if (strcmp(al_get(tokens, 0), "cd") == 0) {
        cd(tokens);
        return 1;
    } else if (strcmp(al_get(tokens, 0), "pwd") == 0) {
        pwd();
        return 1;
    } else if (strcmp(al_get(tokens, 0), "which") == 0) {
        which(tokens);
        return 1;
    }

    return 0;
}

// void execute_command(command* com) {
    
//     // fork
//     pid_t pid = fork();
//     if (pid == -1) {
//         perror("Error forking");
//         set_exit_status(FAILURE);
//     } else if (pid == 0) {
//         // we are inside child process

//         // handle input redirection
        


//     } else {
//         // parent process
//         int status;
//         wait(&status);
//         // handle status
//     }

//     // a > b > c
// }

void parse_and_execute(int fd) {
    input_stream* stream = input_stream_init(fd);
    char* line = read_line(stream);
    arraylist_t* tokens = tokenize(line);

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
    }

    // wildcard expansion
    for (int i = 0; i < al_length(tokens); i++) {
        wildcard_expansion(al_get(tokens, i), tokens, i);
    }

    // find pipeline index. if no pipeline, just create 1 command
    // if pipeline exists, create command for before and after
    int pipelineIndex = -1;
    for (int i = 0; i < al_length(tokens); i++) {
        if (strncmp(al_get(tokens, i), "|", 1) == 0) {
            pipelineIndex = i;
            break;
        }
    }

    al_set(tokens, 0, handle_program_path(al_get(tokens, 0)));

    if (pipelineIndex == -1) {
        //make child process
        pid_t child = fork();
        if (child == -1) {
            perror("Error forking");
            set_exit_status(FAILURE);
        } else if (child == 0) {
            // inside child process
            command* com = command_init(al_get(tokens, 0));
            command_populate(com, tokens, 0, al_length(tokens));

            if (al_length(com->redir_inputs) > 0) {
                int in = open(al_get(com->redir_inputs, al_length(com->redir_inputs) - 1), O_RDONLY);
                if (in == -1) {
                    perror("Error opening input file");
                    set_exit_status(FAILURE);
                }
                dup2(in, STDIN_FILENO);
                close(in);
            }
            
            if (al_length(com->redir_outputs) > 0) {
                int out;
                for (int i = 0; i < al_length(com->redir_outputs); i++) {
                    out = open(al_get(com->redir_outputs, i), O_CREAT | O_WRONLY | O_TRUNC, 0640);
                    if (out == -1) {
                        perror("Error opening output file");
                        set_exit_status(FAILURE);
                    }
                }
                dup2(out, STDOUT_FILENO);
                close(out);
            }

            if (execv(com->program, com->arguments->data) == -1) {
                set_exit_status(FAILURE);
            } else {
                set_exit_status(SUCCESS);
            }
            command_destroy(com);
        } else {
            // inside parent process
            wait(NULL);
        }
    } else {
        int pipefd[2]; // pipefd[0] = read, pipefd[1] = write
        if (pipe(pipefd) == -1) {
            perror("Error creating pipe");
            set_exit_status(FAILURE);
            return;
        }

        // 2 child processes, 1 for each command in the pipe
        pid_t child1 = fork();
        if (child1 == -1) {
            perror("Error forking child1");
            set_exit_status(FAILURE);
        } else if (child1 == 0) {
            command* com1 = command_init(al_get(tokens, 0));
            command_populate(com1, tokens, 0, pipelineIndex);
                
            if (al_length(com1->redir_inputs) > 0) {
                int in = open(al_get(com1->redir_inputs, al_length(com1->redir_inputs) - 1), O_RDONLY);
                if (in == -1) {
                    perror("Error opening input file");
                    set_exit_status(FAILURE);
                }
                dup2(in, STDIN_FILENO);
                close(in);
            }
            
            if (al_length(com1->redir_outputs) > 0) {
                int out;
                for (int i = 0; i < al_length(com1->redir_outputs); i++) {
                    out = open(al_get(com1->redir_outputs, i), O_CREAT | O_WRONLY | O_TRUNC, 0640);
                    if (out == -1) {
                        perror("Error opening output file");
                        set_exit_status(FAILURE);
                    }
                }
                dup2(out, STDOUT_FILENO);
                close(out);
            }
            
            dup2(pipefd[1], STDOUT_FILENO);

            if (execv(com1->program, com1->arguments->data) == -1) {
                set_exit_status(FAILURE);
            } else {
                set_exit_status(SUCCESS);
            }
            command_destroy(com1);
        } 

        pid_t child2 = fork();
        if (child2 == -1) {
            perror("Error forking child2");
            set_exit_status(FAILURE);
        } else if (child2 == 0) {
            command* com2 = command_init(al_get(tokens, 0));
            command_populate(com2, tokens, 0, pipelineIndex);
                
            if (al_length(com2->redir_inputs) > 0) {
                int in = open(al_get(com2->redir_inputs, al_length(com2->redir_inputs) - 1), O_RDONLY);
                if (in == -1) {
                    perror("Error opening input file");
                    set_exit_status(FAILURE);
                }
                dup2(in, STDIN_FILENO);
                close(in);
            }

            dup2(pipefd[0], STDIN_FILENO);
            
            if (al_length(com2->redir_outputs) > 0) {
                int out;
                for (int i = 0; i < al_length(com2->redir_outputs); i++) {
                    out = open(al_get(com2->redir_outputs, i), O_CREAT | O_WRONLY | O_TRUNC, 0640);
                    if (out == -1) {
                        perror("Error opening output file");
                        set_exit_status(FAILURE);
                    }
                }
                dup2(out, STDOUT_FILENO);
                close(out);
            }

            
            if (execv(com2->program, com2->arguments->data) == -1) {
                set_exit_status(FAILURE);
            } else {
                set_exit_status(SUCCESS);
            }
            
            command_destroy(com2);
        }

        close(pipefd[0]);
        close(pipefd[1]);

        wait(NULL);
        wait(NULL);
    }
}


// iterate through all the lines in the file and parse and execute each line
void batch_mode(int fd) {
    char* line;
    
    if (fd == -1) {
        perror("Error opening file");
        set_exit_status(FAILURE);
    }

    /* 
        Iterate through each line
        parse_and_execute each line , only thing is parse_and_execute takes in file descriptor and not the line
    */

        
}



void interactive_mode() {
    printf("%s\n", WELCOME_MSG);
    while (1) {
        printf("%s", PROMPT);
        parse_and_execute(STDIN_FILENO);
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