CC = gcc
CFLAGS = -Wall -O2

all: writer reader

writer: writer.c
	$(CC) $(CFLAGS) -o writer writer.c

reader: reader.c
	$(CC) $(CFLAGS) -o reader reader.c

clean:
	rm -f writer reader
