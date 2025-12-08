
CC = gcc
CFLAGS = -O2 -Wall -pthread
BIN_DIR = bin
SRC_DIR = src
TARGET = $(BIN_DIR)/poly_mult  $(BIN_DIR)/mergesort $(BIN_DIR)/sparse_matrix_vector_mult
SRCS = $(SRC_DIR)/poly_mult.c $(SRC_DIR)/mergesort.c $(SRC_DIR)/sparse_matrix_vector_mult.c

.PHONY: all clean

all: $(TARGET)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(TARGET): $(SRCS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(BIN_DIR)

