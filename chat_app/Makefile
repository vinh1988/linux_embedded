CC = gcc
CFLAGS = -Wall -Wextra -Iinclude

all: server client

server: src/server.c src/common.c
	$(CC) $(CFLAGS) -o server src/server.c src/common.c

client: src/client.c src/common.c
	$(CC) $(CFLAGS) -o client src/client.c src/common.c

clean:
	rm -f server client
