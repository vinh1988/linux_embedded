CC = gcc
CFLAGS = -Wall -Wextra -g -lrt
SRC = src/main.c
OBJ = obj/main.o
BIN = bin/msg_queue

all: $(BIN)

$(BIN): $(OBJ)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $(BIN) $(OBJ)

obj/main.o: src/main.c
	@mkdir -p obj
	$(CC) $(CFLAGS) -c src/main.c -o obj/main.o

clean:
	rm -rf bin obj
