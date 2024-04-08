# MyShell

This project was created by:

1. Rachit Gupta: rg1091
2. Vian Miranda: vsm35

# MyShell Design

In our 'mysh.c' file, we utilized many helper functions to handle different parts of the shell. We also used Professor Menendez's implementation of arraylist that he provided in class.

In our main function, we checked the number of provided arguments and what isatty(STDIN_FILENO) returns to determine what mode we are in.
  1. If there is 1 argument and isatty(STDIN_FILENO) == 1, we are in interactive mode
  2. If there is 1 argument and isatty(STDIN_FILENO) == 0, we are in batch mode with no file provided, indicating there is piping.
  3. If there are 2 arguments, we are in batch mode with a file provided.


Based on this, we created two functions: interactive_mode, and batch_mode(int fd).

Here are some important helper functions that were utilized:

1. execute_command(command* com, int pipeStatus, int pipefd) - here we handle the redirection and input/output for piping using dup2.

2. parse_and_execute(input_stream* stream) - this is where we handle the many parts of a complex shell, including reading, parsing, and executing a command line from the user 
  1. we use read_line(stream) to read the current stream and return it as a char* line
  2. we tokenize(line) to get an arraylist of tokens and free(line) now that we finished using it.
  3. Check for conditionals (then, else). This is the first thing we check for because it would be the first token and takes precedence over the rest of the command.
    a. If we have "then", we check if the exit status of the previous command is SUCCESS. If it is, we remove "then" from tokens list and continue, otherwise stop immediately. If we have "else", we check if exit status is FAILURE - if it is, remove "else" and continue, otherwise stop immediately.
  4. Check for "exit" - if we encounter this, then exit_shell
  5. Handle wildcard expansion. We used glob to do this.
  6. Find the index of "|" (pipelineIndex). This would indicate that there is piping.
  7. Handle piping
    a. If there is no piping:
     We get the program path and use fork() to create to handle 1 child process since there is only 1 command. We execute_command when we are in the child process, and then use waitpid in the parent process to get the status of the child process. 
    b. If there is piping:
      We use pipe(pipefd) to create a pipe. Then we get the program path for both commands and We do the same thing as before except with 2 child processes since there are 2 commands.
  
    


interactive_mode:
  Interactive Mode is where the input comes from the terminal and the user can continue to type commands interactively.
  
  Steps:
  1. The function starts by printing a welcome message to the user. 
  2. An input_stream is initialized with STDIN_FILENO as the file descriptor.
  3. While the input stream is not finished (NULL) and get_exit_flag() is marked as CONTINUE, we print the prompt and parse_and_execute(stream).
  4. Eventually, get_exit_flag() will be marked as EXIT, which at this point we free our input stream and print the exit message.


batch_mode:
  Batch mode is where commands are being read in order from a provided file OR from a piped input when there is no specified file provided.

  Steps:
  1. An input_stream is initialized with STDIN_FILENO if there is no file provided, and with the fd of the file provided if there is one.
  2. While input stream is not finished (NULL) and the file's fd is not = -1, we parse_and_execute(stream). 

# Testing

1. Interacting Mode

  a. Built-in-commands
  a. Conditionals
  b. Redirection
    1. Single redirection
    2. Multiple redirections in a single command
  c. Pipelines
  d. Wildcards
  d. Error checks

2. Batch Mode
  
  a. Testing with file provided
  b. Testing with no file provided (in pipeline)