# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99 -Iinclude

# Source and object files
SRC = $(wildcard src/*.c)
OBJ = $(SRC:src/%.c=build/%.o)

# Output
EXEC = kilo

# Default target
all: $(EXEC)

# Link step
$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(EXEC)

# Compile step
build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure build dir exists
build:
	mkdir -p build

# Clean build
clean:
	rm -rf build $(EXEC)

.PHONY: all clean
