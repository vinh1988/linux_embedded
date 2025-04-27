# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -pthread

# Source files
SRCS = main.c
TARGET = chat

# Default target
all: $(TARGET)

# Compile all source files directly into the final executable
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

# Clean compiled files
clean:
	rm -f $(TARGET)

# Run the program with a default port
run:
	./$(TARGET) 5000
