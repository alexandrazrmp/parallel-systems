CC = gcc
CFLAGS = -O2 -Wall -pthread
BIN_DIR = bin
SRC_DIR = src

# Targets
POLY_MULT_TARGET = $(BIN_DIR)/poly_mult
SHARED_VAR_TARGET = $(BIN_DIR)/shared_var_update

# Sources
POLY_MULT_SRCS = $(SRC_DIR)/poly_mult.c
SHARED_VAR_SRCS = $(SRC_DIR)/shared_var_update.c

.PHONY: all clean

all: $(POLY_MULT_TARGET) $(SHARED_VAR_TARGET)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(POLY_MULT_TARGET): $(POLY_MULT_SRCS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(SHARED_VAR_TARGET): $(SHARED_VAR_SRCS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(BIN_DIR)