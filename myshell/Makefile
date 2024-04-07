CC = gcc

DEBUG ?= 0
ifeq ($(DEBUG), 1)
	CFLAGS = -Wall -Wextra -fsanitize=address -g
else
	CFLAGS = -Wall -Wextra
endif

# List of source files
SRC_FILES = arraylist.c commands.c status.c mysh.c

# List of object files
OBJ_FILES = arraylist.o commands.o status.o mysh.o

# Executables
MYSH_EXEC = mysh

# Default target
all: $(MYSH_EXEC)

# Build spchk executable
$(MYSH_EXEC): arraylist.c commands.c status.c mysh.c
	$(CC) $(CFLAGS) -o $@ $^


# Clean command
clean:
	rm -f $(OBJ_FILES) $(MYSH_EXEC)

