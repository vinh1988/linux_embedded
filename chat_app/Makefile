CC = gcc
CFLAGS = -Wall -Wextra -Iinclude

all: server client help

server: main.o src/server.o src/common.o src/help.o
	$(CC) $(CFLAGS) -o server main.o src/server.o src/common.o src/help.o

client: src/client.o src/common.o src/help.o
	$(CC) $(CFLAGS) -o client src/client.o src/common.o src/help.o

help: src/help.o
	$(CC) $(CFLAGS) -o help src/help.o

# Compiling individual source files
main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.o

src/server.o: src/server.c
	$(CC) $(CFLAGS) -c src/server.c -o src/server.o

src/client.o: src/client.c
	$(CC) $(CFLAGS) -c src/client.c -o src/client.o

src/common.o: src/common.c
	$(CC) $(CFLAGS) -c src/common.c -o src/common.o

src/help.o: src/help.c
	$(CC) $(CFLAGS) -c src/help.c -o src/help.o

clean:
	rm -f server client help main.o src/*.o
