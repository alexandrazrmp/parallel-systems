/*
 * assignment3/src/sparse_matrix_vector_mult.c
 * Exercise 3.2 - Sparse Matrix-Vector Multiplication using MPI
 * Usage: ./bin/sparse_matrix_vector_mult <square-matrix_size> <sparsity%> <iterations>
 *
 * Multiplication of a sparse matrix with a vector using the Compressed Sparse Row (CSR) representation of sparse Matrices
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>


// Function to get the current time in seconds
double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}

int main(int argc, char** argv) {

    //check input
    if (argc != 4) {
        printf("Usage: %s N sparsity iterations\n", argv[0]);
        return 0;
    }

    //read input parameters
    int N = atoi(argv[1]);          // Matrix dimension (NxN)
    int sparsity = atoi(argv[2]); // Percentage of zero elements         probably not needed for processes other than 0
    int iters = atoi(argv[3]);      // Number of SpMV iterations

    if (sparsity < 0 || sparsity > 100) {
        printf("0 <= sparsity <= 100\n");
        return 0;
    }


    srand(time(NULL));  //initialize time

    MPI_Init(&argc, &argv); //initialize MPI with main funnction's arguements??? maybe less arguements

    int rank, size;

    //get ID of this process
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    //get total number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    /* Dense matrix and vectors (only fully used by rank 0) */
    int **A = NULL;
    int *x = NULL;


    // Result vectors
    int *x_serial = NULL, *y_serial = NULL;         // for dense serial
    int *x_csr_serial = NULL, *y_csr_serial = NULL; // for CSR serial
    int *y_csr_parallel = NULL;                     // for CSR parallel
    int *y_dense_parallel = NULL;                   // for dense parallel

    // Timing variables
    double t0, t1;
    double csr_construction_time = 0.0, CSR_message_send_time = 0.0;
    double CSR_mult_time = 0.0, total_CSR = 0.0;
    double dense_message_send_time = 0.0, dense_mult_time = 0.0, total_dense = 0.0;
    double CSR_mult_time_serial = 0.0, total_CSR_serial = 0.0, total_dense_serial = 0.0;


    //rank 0 initializes matrix and vector
    if (rank == 0) {

        A = malloc(N * sizeof(int *));
        x = malloc(N * sizeof(int));

        //initialize vector and matrix with given sparsity
        for (int i = 0; i < N; i++) {
            x[i] = rand() % 1000;   // vector has elements 0..999
            A[i] = malloc(N* sizeof(int));
            for (int j = 0; j < N; j++)
                A[i][j] = ((rand()%100) < sparsity) ? 0 : (rand() % 1000 + 1);
        }
    }


    //serial dense multiplication by rank 0
    if (rank == 0) {
        x_serial = malloc(N * sizeof(int));
        y_serial = malloc(N * sizeof(int));

        // copy initial x into x_serial
        for (int i = 0; i < N; i++)
            x_serial[i] = x[i];

        t0 = get_time();

        for (int it = 0; it < iters; it++) {
            for (int i = 0; i < N; i++) {
                int sum = 0;
                for (int j = 0; j < N; j++) {
                    sum += A[i][j] * x_serial[j];
                }
                y_serial[i] = sum;
            }

            // result becomes input for next iteration
            for (int i = 0; i < N; i++)
                x_serial[i] = y_serial[i];
        }

        t1 = get_time();
        total_dense_serial = t1 - t0;
    }


    //CSR construction by rank 0
    int nnz = 0;           // number of non-zero elements
    int *row_ptr = NULL;    // row pointer array
    int *col_idx = NULL;    // column indices
    int *values = NULL;     // non-zero values

    if (rank == 0) {
        t0 = get_time();  // start CSR timing

        // First, count non-zero elements
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                if (A[i][j] != 0)
                    nnz++;
            }
        }

        // Allocate CSR arrays
        row_ptr = malloc((N + 1) * sizeof(int));
        col_idx = malloc(nnz * sizeof(int));
        values = malloc(nnz * sizeof(int));

        int idx = 0;
        row_ptr[0] = 0;

        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                if (A[i][j] != 0) {
                    values[idx] = A[i][j];
                    col_idx[idx] = j;
                    idx++;
                }
            }
            row_ptr[i + 1] = idx;  // next row starts here
        }

        t1 = get_time();  // end CSR timing
        csr_construction_time = t1 - t0;
    }


    //serial CSR multiplication
    if (rank == 0) {
        // Allocate temporary vectors
        x_csr_serial = malloc(N * sizeof(int));
        y_csr_serial = malloc(N * sizeof(int));

        // Copy initial vector
        for (int i = 0; i < N; i++)
            x_csr_serial[i] = x[i];

        t0 = get_time();  // start timing

        for (int it = 0; it < iters; it++) {
            for (int i = 0; i < N; i++) {
                int sum = 0;
                // Iterate only over non-zero entries using CSR
                for (int k = row_ptr[i]; k < row_ptr[i+1]; k++) {
                    sum += values[k] * x_csr_serial[col_idx[k]];
                }
                y_csr_serial[i] = sum;
            }

            // result becomes input for next iteration
            for (int i = 0; i < N; i++)
                x_csr_serial[i] = y_csr_serial[i];
        }

        t1 = get_time();  // end timing
        CSR_mult_time_serial = t1 - t0;
    }


    //parallel CSR message send (OPTIMAL)

    MPI_Barrier(MPI_COMM_WORLD);
    t0 = get_time();

    //decide row ownership 
    int rows = N / size;
    int start = rank * rows;
    int end   = (rank == size - 1) ? N : start + rows;
    int local_rows = end - start;

    //prepare distribution metadata on rank 0
    int *nnz_counts = NULL, *nnz_displs = NULL;

    if (rank == 0) {
        nnz_counts = malloc(size * sizeof(int));
        nnz_displs = malloc(size * sizeof(int));

        for (int p = 0; p < size; p++) {
            int s = p * rows;
            int e = (p == size - 1) ? N : s + rows;

            nnz_counts[p] = row_ptr[e] - row_ptr[s];
            nnz_displs[p] = row_ptr[s];
        }
    }

    //scatter nnz counts so each process knows its local nnz
    int local_nnz;
    MPI_Scatter(nnz_counts, 1, MPI_INT,
                &local_nnz, 1, MPI_INT,
                0, MPI_COMM_WORLD);

    //allocate local CSR
    int *local_row_ptr = malloc((local_rows + 1) * sizeof(int));
    int *local_col_idx = malloc(local_nnz * sizeof(int));
    int *local_values  = malloc(local_nnz * sizeof(int));

    // Scatter CSR data (values and col_idx)
    MPI_Scatterv(values, nnz_counts, nnz_displs, MPI_INT,
                local_values, local_nnz, MPI_INT,
                0, MPI_COMM_WORLD);

    MPI_Scatterv(col_idx, nnz_counts, nnz_displs, MPI_INT,
                local_col_idx, local_nnz, MPI_INT,
                0, MPI_COMM_WORLD);


    // Send corresponding row_ptr slices manually
    if (rank == 0) {
        for (int p = 0; p < size; p++) {
            int s = p * rows;
            int e = (p == size - 1) ? N : s + rows;
            int count = e - s + 1;
            if (p == 0) {
                memcpy(local_row_ptr, &row_ptr[s], count * sizeof(int));
            } else {
                MPI_Send(&row_ptr[s], count, MPI_INT, p, 0, MPI_COMM_WORLD);
            }
        }
    } else {
        MPI_Recv(local_row_ptr, local_rows + 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    //fix row_ptr offsets locally
    int base = local_row_ptr[0];
    for (int i = 0; i <= local_rows; i++)
        local_row_ptr[i] -= base;

    //Broadcast vector x to all processes since its the same
    if (rank != 0)
        x = malloc(N * sizeof(int));

    MPI_Bcast(x, N, MPI_INT, 0, MPI_COMM_WORLD);

    t1 = get_time();
    CSR_message_send_time = t1 - t0;


    //parallel CSR multiplication

    MPI_Barrier(MPI_COMM_WORLD);
    t0 = get_time();

    /* --- Allocate local result vector --- */
    int *local_y = malloc(local_rows * sizeof(int));
    int *x_next = malloc(N * sizeof(int));  // temp vector for next iteration

    int *x_parallel_csr = malloc(N * sizeof(int));
    memcpy(x_parallel_csr, x, N * sizeof(int));


    // Prepare gather metadata ONCE (not every iteration)
    int *recv_counts = malloc(size * sizeof(int));
    int *recv_displs = malloc(size * sizeof(int));

    for (int p = 0; p < size; p++) {
        int s = p * rows;
        int e = (p == size - 1) ? N : s + rows;
        recv_counts[p] = e - s;
        recv_displs[p] = s;
    }
    /* --- Start iterations --- */
    for (int it = 0; it < iters; it++) {

        // Compute local SpMV
        for (int i = 0; i < local_rows; i++) {
            int sum = 0;
            for (int k = local_row_ptr[i]; k < local_row_ptr[i+1]; k++) {
                sum += local_values[k] * x_parallel_csr[local_col_idx[k]];
            }
            local_y[i] = sum;
        }

        // /* --- Gather all local results to rank 0 --- */
        // // use MPI_Gatherv for different row sizes (last process may have more rows)
        // int *recv_counts = NULL;
        // int *recv_displs = NULL;
        // if (rank == 0) {
        //     recv_counts = malloc(size * sizeof(int));
        //     recv_displs = malloc(size * sizeof(int));
        //     for (int p = 0; p < size; p++) {
        //         int s = p * rows;
        //         int e = (p == size-1) ? N : s + rows;
        //         recv_counts[p] = e - s;
        //         recv_displs[p] = s;
        //     }
        // }

        // MPI_Gatherv(local_y, local_rows, MPI_INT,
        //             x_next, recv_counts, recv_displs, MPI_INT,
        //             0, MPI_COMM_WORLD);

        // /* --- Broadcast updated vector for next iteration --- */
        // MPI_Bcast(x_next, N, MPI_INT, 0, MPI_COMM_WORLD);

        // // swap pointers for next iteration
        // int *tmp = x_parallel_csr;
        // x_parallel_csr = x_next;
        // x_next = tmp;



        /* --- All processes collect the full vector --- */
        MPI_Allgatherv(local_y, local_rows, MPI_INT,
                    x_next, recv_counts, recv_displs, MPI_INT,
                    MPI_COMM_WORLD);

        int *tmp = x_parallel_csr;
        x_parallel_csr = x_next;
        x_next = tmp;


    }

    t1 = get_time();
    CSR_mult_time = t1 - t0;

    free(recv_counts);
    free(recv_displs);


    if (rank == 0) {
        y_csr_parallel = malloc(N * sizeof(int));
        memcpy(y_csr_parallel, x_parallel_csr, N * sizeof(int));
    }

    // CSR local arrays
    free(local_row_ptr);
    free(local_col_idx);
    free(local_values);
    free(local_y);
    free(x_next);
    free(x_parallel_csr);


    //parallel dense message send

    MPI_Barrier(MPI_COMM_WORLD);
    t0 = get_time();

    // Decide row ownership
     rows = N / size;
     start = rank * rows;
     end   = (rank == size - 1) ? N : start + rows;
    int local_dense_rows = end - start;

    int *local_A_flat = malloc(local_dense_rows * N * sizeof(int));
    int *dense_row_counts = NULL;
    int *dense_row_displs = NULL;
    int *flat_A = NULL;

    if (rank == 0) {
        flat_A = malloc(N * N * sizeof(int));
        for (int i = 0; i < N; i++)
            memcpy(&flat_A[i * N], A[i], N * sizeof(int));

        dense_row_counts = malloc(size * sizeof(int));
        dense_row_displs = malloc(size * sizeof(int));
        for (int p = 0; p < size; p++) {
            int s = p * rows;
            int e = (p == size - 1) ? N : s + rows;
            dense_row_counts[p] = (e - s) * N;
            dense_row_displs[p] = s * N;
        }
    }

    MPI_Scatterv(flat_A,
                dense_row_counts,
                dense_row_displs,
                MPI_INT,
                local_A_flat,
                local_dense_rows * N,
                MPI_INT,
                0,
                MPI_COMM_WORLD);

    if (rank == 0) free(flat_A);
    if (rank != 0 && x == NULL) x = malloc(N * sizeof(int));
    MPI_Bcast(x, N, MPI_INT, 0, MPI_COMM_WORLD);

    t1 = get_time();
    dense_message_send_time = t1 - t0;

    //parallel dense multiplication
    MPI_Barrier(MPI_COMM_WORLD);
    t0 = get_time();

    int *local_y_dense = malloc(local_dense_rows * sizeof(int));
    int *x_next_dense = malloc(N * sizeof(int));

    if (rank == 0) {
        y_dense_parallel = malloc(N * sizeof(int));
    }

    memcpy(x_next_dense, x, N * sizeof(int));

    for (int it = 0; it < iters; it++) {
        for (int i = 0; i < local_dense_rows; i++) {
            int sum = 0;
            for (int j = 0; j < N; j++)
                sum += local_A_flat[i * N + j] * x[j];
            local_y_dense[i] = sum;
        }

        int *recv_counts_dense = NULL;
        int *recv_displs_dense = NULL;
        if (rank == 0) {
            recv_counts_dense = malloc(size * sizeof(int));
            recv_displs_dense = malloc(size * sizeof(int));
            for (int p = 0; p < size; p++) {
                int s = p * rows;
                int e = (p == size - 1) ? N : s + rows;
                recv_counts_dense[p] = e - s; // number of rows
                recv_displs_dense[p] = s;
            }
        }

        MPI_Gatherv(local_y_dense,
                    local_dense_rows,
                    MPI_INT,
                    rank == 0 ? y_dense_parallel : NULL,
                    recv_counts_dense,
                    recv_displs_dense,
                    MPI_INT,
                    0,
                    MPI_COMM_WORLD);

        if (rank == 0)
            memcpy(x_next_dense, y_dense_parallel, N * sizeof(int));

        MPI_Bcast(x_next_dense, N, MPI_INT, 0, MPI_COMM_WORLD);

        int *tmp = x;
        x = x_next_dense;
        x_next_dense = tmp;

        if (rank == 0) {
            free(recv_counts_dense);
            free(recv_displs_dense);
        }
    }

    t1 = get_time();
    dense_mult_time = t1 - t0;

    free(local_A_flat);
    free(local_y_dense);
    free(x_next_dense);
    if (rank == 0) {
        free(dense_row_counts);
        free(dense_row_displs);
    }



    //rank 0 prints results and frees allocated memory
    if (rank == 0) {
        total_CSR = csr_construction_time + CSR_message_send_time + CSR_mult_time;
        total_CSR_serial = csr_construction_time + CSR_mult_time_serial;
        total_dense = dense_message_send_time + dense_mult_time;

        printf("CSR construction time: %.6f s\n", csr_construction_time);
        printf("CSR message send time: %.6f s\n", CSR_message_send_time);
        printf("CSR parallel multiplication time: %.6f s\n", CSR_mult_time);
        printf("Total parallel CSR time: %.6f s\n", total_CSR);
        printf("Total serial CSR time: %.6f s\n", total_CSR_serial);
        printf("Total parallel dense time: %.6f s\n", total_dense);
        printf("Total serial dense time: %.6f s\n", total_dense_serial);

        int correct = 1;
        for (int i = 0; i < N; i++) {
            if (y_serial[i] != y_csr_serial[i] ||
                y_serial[i] != y_dense_parallel[i] ||
                y_serial[i] != y_csr_parallel[i]) {
                correct = 0;
                break;
            }
        }
        if (correct) 
            printf("All results are correct!\n");
        else
            printf("ERROR: Results do not match!\n");


        //free allocated memory

        free(y_serial);
        free(x_serial);

        free(y_csr_serial);
        free(x_csr_serial);

        free(row_ptr);
        free(col_idx);
        free(values);

        free(y_csr_parallel);
        free(y_dense_parallel);

        for (int i = 0; i < N; i++) free(A[i]);
        free(A);

        free(x);

    }


    /* Finalize MPI */
    MPI_Finalize();
    return 0;
}
