# MyShell

This project was created by:

1. Rachit Gupta: rg1091
2. Vian Miranda: vsm35

# MyShell Design

In our 'mysh.c' file, we utilized many helper functions to handle different parts of the shell. We also used Professor Menendez's implementation of arraylist that he provided in class.

In our main function, we checked the number of provided arguments and what isatty(STDIN_FILENO) returns to determine what mode we are in.
  1. If there is 1 argument and `isatty(STDIN_FILENO) == 1`, we are in interactive mode
  2. If there is 1 argument and `isatty(STDIN_FILENO) == 0`, we are in batch mode with no file provided, indicating there is piping.
  3. If there are 2 arguments, we are in batch mode with a file provided.


Based on this, we created two functions: interactive_mode, and batch_mode(int fd).

Here are some important helper functions that were utilized:

1. execute_command(command* com, int pipeStatus, int pipefd) - here we handle the redirection and input/output for piping using `dup2`.

2. parse_and_execute(input_stream* stream) - this is where we handle the many parts of a complex shell, including reading, parsing, and executing a command line from the user 
  1. We use `read_line(stream)` to read the current stream and return it as a char* line
  2. We `tokenize(line)` to get an arraylist of tokens and `free(line)` now that we finished using it.
  3. Check for conditionals (then, else). This is the first thing we check for because it would be the first token and takes precedence over the rest of the command.
    a. If we have "then", we check if the exit status of the previous command is SUCCESS. If it is, we remove "then" from tokens list and continue, otherwise stop immediately. If we have "else", we check if exit status is FAILURE - if it is, remove "else" and continue, otherwise stop immediately.
  4. Check for "exit" - if we encounter this, then `exit_shell`
  5. Handle wildcard expansion. We used glob to do this.
  6. Find the index of "|" (pipelineIndex). This would indicate that there is piping.
  7. Handle piping
    a. If there is no piping:
     We get the program path and use `fork()` to create to handle 1 child process since there is only 1 command. We execute_command when we are in the child process, and then use waitpid in the parent process to get the status of the child process. 
    b. If there is piping:
      We use `pipe(pipefd)` to create a pipe. Then we get the program path for both commands and We do the same thing as before except with 2 child processes since there are 2 commands.
  
    


## interactive_mode:
  Interactive Mode is where the input comes from the terminal and the user can continue to type commands interactively.
  
  Steps:
  1. The function starts by printing a welcome message to the user. 
  2. An input_stream is initialized with STDIN_FILENO as the file descriptor.
  3. While the input stream is not finished (NULL) and get_exit_flag() is marked as CONTINUE, we print the prompt and parse_and_execute(stream).
  4. Eventually, get_exit_flag() will be marked as EXIT, which at this point we free our input stream and print the exit message.


## batch_mode:
  Batch mode is where commands are being read in order from a provided file OR from a piped input when there is no specified file provided.

  Steps:
  1. An input_stream is initialized with STDIN_FILENO if there is no file provided, and with the fd of the file provided if there is one.
  2. While input stream is not finished (NULL) and the file's fd is not = -1, we parse_and_execute(stream). 

# Testing

### I. Interactive Mode

  1. Built-in-commands (cd, pwd, which, exit)
      ```
      Welcome to mysh!
      mysh> pwd
      /common/home/vsm35/Documents/systems214/systems-214/myshell
      mysh> cd
      Error: cd requires an argument
      mysh> cd testenv
      mysh> pwd
      /common/home/vsm35/Documents/systems214/systems-214/myshell/testenv
      mysh> cd hi
      Error using cd: No such file or directory  
      mysh>
      ```
      Here we tested cd and pwd to work. 'testenv' is a folder inside 'myshell', and as we can see, both commands work correctly. There is no 'hi' folder in 'testenv', so the error message is shown successfully.


      ```
      Welcome to mysh!
      mysh> which ls
      /usr/bin/ls
      mysh> exit we are exiting
      we are exiting 
      Exiting mysh...
      ```
      Here we tested which and exit. '/usr/bin/ls' is returned because it is the directory where ls comes from. 'we are exiting' is printed because that is what we had after the exit message, and 'Exiting mysh...' is printed to indicate that we are done with the program.

  2. Conditionals
  
    a. then
    
      ```
      Welcome to mysh!
      mysh> cd hii
      Error using cd: No such file or directory
      mysh> then cd testenv
      mysh> pwd
      /common/home/vsm35/Documents/systems214/systems-214/myshell
      mysh> then cd testenv
      mysh> pwd
      /common/home/vsm35/Documents/systems214/systems-214/myshell/testenv
      mysh>  
      ```


    b. else
    
      ```
      mysh> cd hii
      Error using cd: No such file or directory
      mysh> else cd testenv
      mysh> pwd
      /common/home/vsm35/Documents/systems214/systems-214/myshell/testenv
      mysh>
      ```

  3. Redirection
  
    a. Single redirection
      ```
      echo "Hello, World" > output.txt
      mysh> cat output.txt
      "Hello, World"
      mysh> sort <output.txt
      "Hello, World"
      ```

    b. Multiple redirections in a single command
      ```
      mysh> cat thing.txt
      gg
      aa
      ll
      mysh> cat output.txt
      "Hello, World"
      mysh> sort < thing.txt > output.txt
      mysh> cat output.txt
      aa
      gg
      ll
      mysh> cat output.txt > file1.txt > file2.txt            
      mysh> cat file1.txt
      mysh> cat file2.txt
      aa
      gg
      ll
      mysh> 
      ```
      In this example, the contents of 'thing.txt' and 'output.txt' are printed. Once printed, the input for `sort` is set to 'thing.txt'. The sorted output is then written to 'output.txt', where the original contents are truncated. Then we print the `cat` the contents of 'output.txt' to 'file1.txt' and 'file2.txt'. This creates both output files, but only prints to the last one, 'file2.txt', in accordance with how bash handles multiple inputs/outputs.

  4. Pipelines

    Below is what is inside the file 'pipelinein.txt'
      ```
      I am cool.
      I am smart.
      He is kind.
      He is smart.
      He is smart.
      ```

    Here is the pipeline command:
      ```
      mysh> sort pipelinein.txt | uniq > pipelineout.txt
      ```

    This is what is inside pipelineout.txt:
      ```
      mysh> cat pipelineout.txt
      He is kind.
      He is smart.
      I am cool.
      I am smart.
      ```

    As you can see, sort pipelinein.txt sorts the lines inside it, and that is used as input for 'pipelineout.txt' as part of the pipe. 

    Another example: 
      ```
      mysh> cat < output.txt | grep Hello
      "Hello, World"
      ```

  5. Precedence for redirection in pipeline
  
    Example for input: 
      ```
      mysh> cat output.txt
      aa
      gg
      ll
      mysh> ls | grep hello < output.txt
      mysh>      
      ```

      This does not output anything and shows how `grep hello < output.txt` takes precedence in this situation because of the '<'. There is no hello in 'output.txt', and thus nothing is printed.

    Example for output:  
      ```
      mysh> ls > printer.txt | grep hello
      mysh> cat printer.txt
      arraylist.c
      arraylist.h
      commands.c
      commands.h
      Makefile
      mysh
      mysh.c
      README.md
      status.c
      status.h
      testenv
      ```
    From this, we can ensure that `ls > printer.txt` takes priority over `grep hello` because the output of `ls` is redirected to 'printer.txt' before it can be piped to `grep hello`

  6. Wildcards

    ```
    mysh> cd testenv
    mysh> ls
    a  file1.txt  file2.txt  glob.c  lol.txt  myscript.sh  output.txt  pipelinein.txt  printer.txt  thing.txt  unique.txt
    mysh> ls *.txt
    file1.txt  file2.txt  lol.txt  output.txt  pipelinein.txt  printer.txt  thing.txt  unique.txt
    mysh>
    ```

    With *, only the txt files are printed.

### II. Batch Mode

  1. Testing with file provided
  ``` bash
  $ cat myscript.sh
  echo testenv/lol.txt
  ls
  cat testenv/lol.txt | cat
  cat testenv/lol.txt | echo
  echo exit
  else echo else 1
  else echo else 2

  $ ./mysh testenv/myscript.sh
  hello worlds
  arraylist.c  arraylist.h  commands.c  commands.h  Makefile  mysh  mysh.c  README.md  status.c  status.h  testenv
  testenv/lol.txt
  arraylist.c  arraylist.h  commands.c  commands.h  Makefile  mysh  mysh.c  README.md  status.c  status.h  testenv
  testenv/thing.txt

  exit
  ```

  The commands are read and executed successfully from the file provided.

  2. Testing with no file provided (in pipeline)

  ``` bash
  $ cat testenv/myscript.sh | ./mysh
  hello worlds
  arraylist.c  arraylist.h  commands.c  commands.h  Makefile  mysh  mysh.c  README.md  status.c  status.h  testenv
  testenv/lol.txt
  arraylist.c  arraylist.h  commands.c  commands.h  Makefile  mysh  mysh.c  README.md  status.c  status.h  testenv
  testenv/thing.txt

  exit
  ```

  `mysh` is ran successfully with the contents of 'myscript.sh'. 

For all commands, we also tested to make sure spacing doesn't make a difference where it shouldn't. Example: `echo "Hello, World" > output.txt` should do the same thing as `echo "Hello, World" >output.txt` (note the no space between > and output the 2nd time)