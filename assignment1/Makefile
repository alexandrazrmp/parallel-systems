
CC = gcc
CFLAGS = -O2 -Wall -pthread
BIN_DIR = bin
SRC_DIR = src

.PHONY: all clean poly_mult array_stats

# Build both binaries: poly_mult and array_stats
ALL_TARGETS = $(BIN_DIR)/poly_mult $(BIN_DIR)/array_stats

all: $(ALL_TARGETS)

# convenience targets to build individual executables
poly_mult: $(BIN_DIR)/poly_mult

array_stats: $(BIN_DIR)/array_stats

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/poly_mult: $(SRC_DIR)/poly_mult.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(BIN_DIR)/array_stats: $(SRC_DIR)/array_stats.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(BIN_DIR)

