
# make is a build automation tool. It reads a Makefile which tells it how to compile and link your program.
# Makefiles are used to only re-build the part of your program that has changed,  so it compiles faster.

# Compiler to use and flags to pass
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -g

# Source files
SRC = src/chatgpt.c src/helper.c # helper.c is an example dependency of chatgpt.c
OBJ = $(SRC:.c=.o) # replace the .c suffix with .o for each item in $(SRC)
TARGET = build/chatgpt # final executable file

# Default target
all: $(TARGET)

# Link object files into executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

# Compile .c to .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJ) $(TARGET)
