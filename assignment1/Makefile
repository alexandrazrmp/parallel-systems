
CC = gcc
CFLAGS = -O2 -Wall -pthread
BIN_DIR = bin
SRC_DIR = src

.PHONY: all clean poly_mult array_stats barrier_pthread barrier_condvar

# Build binaries: poly_mult, array_stats and barrier tests
ALL_TARGETS = $(BIN_DIR)/poly_mult $(BIN_DIR)/array_stats $(BIN_DIR)/barrier_pthread $(BIN_DIR)/barrier_condvar

all: $(ALL_TARGETS)

# convenience targets to build individual executables
poly_mult: $(BIN_DIR)/poly_mult

array_stats: $(BIN_DIR)/array_stats

barrier_pthread: $(BIN_DIR)/barrier_pthread

barrier_condvar: $(BIN_DIR)/barrier_condvar

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/poly_mult: $(SRC_DIR)/poly_mult.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(BIN_DIR)/array_stats: $(SRC_DIR)/array_stats.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(BIN_DIR)/barrier_pthread: $(SRC_DIR)/barriers/barrier_pthread.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(BIN_DIR)/barrier_condvar: $(SRC_DIR)/barriers/barrier_condvar.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(BIN_DIR)

