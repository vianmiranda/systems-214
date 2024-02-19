CC = gcc

DEBUG ?= 0
ifeq ($(DEBUG), 1)
	CFLAGS = -Wall -Wextra -fsanitize=address -g
else
	CFLAGS = -Wall -Wextra
endif

# List of source files
SRC_FILES = memtest.c memgrind.c mymalloc.c

# List of object files
OBJ_FILES = memtest.o memgrind.o mymalloc.o

# Executables
MEMTEST_EXEC = memtest
MEMGRIND_EXEC = memgrind

# Default target
all: $(MEMTEST_EXEC) $(MEMGRIND_EXEC)

# Build memtest executable
$(MEMTEST_EXEC): memtest.c mymalloc.c
	$(CC) $(CFLAGS) -o $@ $^

# Build memgrind executable
$(MEMGRIND_EXEC): memgrind.c mymalloc.c
	$(CC) $(CFLAGS) -o $@ $^

# Clean command
clean:
	rm -f $(OBJ_FILES) $(MEMTEST_EXEC) $(MEMGRIND_EXEC)

