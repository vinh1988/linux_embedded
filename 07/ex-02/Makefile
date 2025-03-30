CC = gcc
CFLAGS = -Wall -Wextra -g
SRC = src/main.c
OBJ = obj/main.o
BIN = bin/pipe_program

all: $(BIN)

$(BIN): $(OBJ)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $(BIN) $(OBJ)

obj/main.o: src/main.c
	@mkdir -p obj
	$(CC) $(CFLAGS) -c src/main.c -o obj/main.o

clean:
	rm -rf bin obj
