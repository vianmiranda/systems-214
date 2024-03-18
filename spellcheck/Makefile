CC = gcc

DEBUG ?= 0
ifeq ($(DEBUG), 1)
	CFLAGS = -Wall -Wextra -fsanitize=address -g
else
	CFLAGS = -Wall -Wextra
endif

# List of source files
SRC_FILES = spchk.c triedict.c

# List of object files
OBJ_FILES = spchk.o triedict.o

# Executables
SPCHK_EXEC = spchk

# Default target
all: $(SPCHK_EXEC)

# Build spchk executable
$(SPCHK_EXEC): spchk.c triedict.c
	$(CC) $(CFLAGS) -o $@ $^


# Clean command
clean:
	rm -f $(OBJ_FILES) $(SPCHK_EXEC)

