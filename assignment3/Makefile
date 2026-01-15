MPICC = mpicc
CFLAGS = -O2 -Wall

BIN_DIR = bin
SRC_DIR = src

TARGETS = $(BIN_DIR)/poly_mult \
          $(BIN_DIR)/sparse_matrix_vector_mult \
		  $(BIN_DIR)/mpihello \
          $(BIN_DIR)/sparse_matrix_vector_mult_old

all: $(TARGETS)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/mpihello: $(SRC_DIR)/mpihello.c | $(BIN_DIR)
	$(MPICC) $(CFLAGS) -o $@ $<

$(BIN_DIR)/poly_mult: $(SRC_DIR)/poly_mult.c | $(BIN_DIR)
	$(MPICC) $(CFLAGS) -o $@ $<

$(BIN_DIR)/sparse_matrix_vector_mult: $(SRC_DIR)/sparse_matrix_vector_mult.c | $(BIN_DIR)
	$(MPICC) $(CFLAGS) -o $@ $<

$(BIN_DIR)/sparse_matrix_vector_mult_old: $(SRC_DIR)/sparse_matrix_vector_mult_old.c | $(BIN_DIR)
	$(MPICC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(BIN_DIR)/*
