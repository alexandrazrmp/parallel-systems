CC = gcc
CFLAGS = -O2 -Wall -fopenmp
BIN_DIR = bin
SRC_DIR = src
RESULTS_DIR = results

TARGETS =	$(BIN_DIR)/sparse_matrix_vector_mult
#			$(BIN_DIR)/poly_mult \
#           $(BIN_DIR)/mergesort

.PHONY: all clean

all: $(BIN_DIR) $(TARGETS)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# $(BIN_DIR)/poly_mult: $(SRC_DIR)/poly_mult.c
# 	$(CC) $(CFLAGS) $< -o $@

# $(BIN_DIR)/mergesort: $(SRC_DIR)/mergesort.c
# 	$(CC) $(CFLAGS) $< -o $@

$(BIN_DIR)/sparse_matrix_vector_mult: $(SRC_DIR)/sparse_matrix_vector_mult.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(BIN_DIR)
	rm -rf $(RESULTS_DIR)
