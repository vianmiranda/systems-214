CC = gcc
CFLAGS = -Wall -Wextra

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
$(MEMTEST_EXEC): memtest.o mymalloc.o
	$(CC) $(CFLAGS) -o $@ $^

# Build memgrind executable
$(MEMGRIND_EXEC): memgrind.o mymalloc.o
	$(CC) $(CFLAGS) -o $@ $^

# Build object files
%.o: %.c
	$(CC) $(CFLAGS) -c $<

# Clean command
clean:
	rm -f $(OBJ_FILES) $(MEMTEST_EXEC) $(MEMGRIND_EXEC)

