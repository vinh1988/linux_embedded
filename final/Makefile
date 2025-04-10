CC = gcc
CFLAGS = -Wall -Wextra -g -pthread
LDFLAGS = -pthread -lsqlite3 -lrt

SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build

SERVER_SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/connection_manager.c $(SRC_DIR)/data_manager.c $(SRC_DIR)/storage_manager.c
LOG_SRCS = $(SRC_DIR)/log_process.c

SERVER_OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SERVER_SRCS))
LOG_OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(LOG_SRCS))

SERVER_TARGET = server
LOG_TARGET = log_process

.PHONY: all clean

all: $(BUILD_DIR) $(SERVER_TARGET) $(LOG_TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(SERVER_TARGET): $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(LOG_TARGET): $(LOG_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c -o $@ $<

clean:
	rm -rf $(BUILD_DIR) $(SERVER_TARGET) $(LOG_TARGET) *.db gateway.log logFifo

run: all
	./$(SERVER_TARGET) 1234
