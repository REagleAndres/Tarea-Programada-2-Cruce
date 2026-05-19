CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2 -g
LIBS = -lpthread

SRC_DIR = src
BIN_DIR = bin
TARGET = $(BIN_DIR)/programa
SOURCES = $(SRC_DIR)/cruce.c
OBJECTS = $(SOURCES:.c=.o)

.PHONY: all run run_fase_1 run_fase_2 clean

all: $(TARGET)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	./$(TARGET)

run_fase_1: all
	./$(TARGET) fase_1

run_fase_2: all
	./$(TARGET) fase_2

clean:
	rm -f $(SRC_DIR)/*.o
	rm -rf $(BIN_DIR)
