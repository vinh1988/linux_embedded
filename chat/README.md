# Chat Application

This is a simple peer-to-peer chat application that allows users to connect to each other and send messages.

## Dependencies

-   stdio.h
-   stdlib.h
-   string.h
-   unistd.h
-   arpa/inet.h
-   sys/types.h
-   sys/socket.h
-   sys/select.h
-   netinet/in.h
-   netdb.h
-   pthread.h

## Building the application

To build the application, run the following command:

```
gcc main.c -o chat -lpthread
```

## Running the application

To run the application, run the following command:

```
./chat <port>
```

where `<port>` is the port number that the application will listen on.

## Commands

The following commands can be used in the chat application:

-   `help`: Show available commands
-   `myip`: Display the IP address
-   `myport`: Display the listening port
-   `connect <IP> <port>`: Connect to another peer
-   `list`: Show all active connections
-   `terminate <id>`: Close a connection
-   `send <id> <message>`: Send a message to a peer
-   `exit`: Close all connections and terminate
