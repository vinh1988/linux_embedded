CC = gcc
CFLAGS = -Wall -O2 -pthread
INCLUDES = -Iinclude

SRC = src
TESTS = tests

all: writer reader

writer: $(SRC)/writer.c include/shared.h
	$(CC) $(CFLAGS) $(INCLUDES) -o writer $(SRC)/writer.c

reader: $(SRC)/reader.c include/shared.h
	$(CC) $(CFLAGS) $(INCLUDES) -o reader $(SRC)/reader.c

test: $(TESTS)/test_shared.c include/shared.h
	$(CC) $(CFLAGS) $(INCLUDES) -o test_shared $(TESTS)/test_shared.c
	./test_shared

clean:
	rm -f writer reader test_shared
