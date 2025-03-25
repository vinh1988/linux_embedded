CC = gcc
CFLAGS = -Wall -Wextra -Iinclude

# Object files
SERVER_OBJS = src/server.o src/common.o src/help.o
CLIENT_OBJS = src/client.o src/common.o src/help.o

# Targets
all: server client

server: server_main.o $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o server server_main.o $(SERVER_OBJS)

client: client_main.o $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o client client_main.o $(CLIENT_OBJS)

# Compilation rules
server_main.o: server_main.c
	$(CC) $(CFLAGS) -c server_main.c -o server_main.o

client_main.o: client_main.c
	$(CC) $(CFLAGS) -c client_main.c -o client_main.o

src/server.o: src/server.c
	$(CC) $(CFLAGS) -c src/server.c -o src/server.o

src/client.o: src/client.c
	$(CC) $(CFLAGS) -c src/client.c -o src/client.o

src/common.o: src/common.c
	$(CC) $(CFLAGS) -c src/common.c -o src/common.o

src/help.o: src/help.c
	$(CC) $(CFLAGS) -c src/help.c -o src/help.o

clean:
	rm -f server client *.o src/*.o
