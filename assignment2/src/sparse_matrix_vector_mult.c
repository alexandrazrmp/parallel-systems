/*
 * assignment2/src/sparse_matrix_vector_mult.c
 * Exercise 2.2 - Sparse Matrix-Vector Multiplication
 * Usage: ./bin/sparse_matrix_vector_mult <square-matrix_size> <sparsity%> <iterations> <threads>
 *
 * Multiplication of a sparse matrix with a vector using the Compressed Sparse Row (CSR) representation of sparse Matrices
 */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <sys/time.h>
#include <string.h>
#include <time.h>

// Function to get the current time in seconds
double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}

int main(int argc, char *argv[]) {

    if (argc != 5) {
        printf("Usage: %s N sparsity iterations threads\n", argv[0]);
        return 1;
    }

    int N         = atoi(argv[1]);   //array size
    int sparsity  = atoi(argv[2]);   //zero-element percentage
    int iterations     = atoi(argv[3]);   //iterations
    int threads   = atoi(argv[4]);   //thread count

    if (N <= 0 || sparsity < 0 || sparsity > 100 || iterations <= 0 || threads <= 0) {
        printf("Wrong Input\n");
        return 1;
    }

    srand(time(NULL));

    //generate random matrix and random vector
    int **a;
    a = malloc(N*sizeof(int*));
    if (a == NULL) {
        printf("Memory allocation failed for matrix rows\n");
        return 1;
    }
    for (int i = 0 ; i < N ; i++) {
        a[i] = malloc(N*sizeof(int));
        if (a[i] == NULL) {
            printf("Memory allocation failed for matrix row %d\n", i);
            return 1;
        }
    }
    
    int *x;
    x = malloc(N*sizeof(int));
    if (x == NULL) {
        printf("Memory allocation failed for vector x\n");
        return 1;
    }

    //fill in random matrix according to sparsity percentage
    //also fill in vector x
    for (int i = 0; i < N; i++) {
        x[i] = rand() % 1000;
        for (int j = 0; j < N; j++) {
            int r = rand() % 100;
            if (r < sparsity)
                a[i][j] = 0;
            else
                a[i][j] = rand() % 1000 + 1;
        }
    }

    printf("N = %d\n", N);
    printf("Sparsity = %d%%\n", sparsity);
    printf("Iterations = %d\n", iterations);
    omp_set_num_threads(threads);                   //have not calculated the time this takes
    printf("OpenMP using %d threads\n", threads);

    double t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11;

    ///////////CSR formation/////////////

    //serial sparse matrix to CSR conversion
    t0 = get_time();

    int *nze_serial = malloc(N * sizeof(int));
    if (nze_serial == NULL) {
        printf("Memory allocation failed for nze_serial\n");
        return 1;
    }
    for (int i = 0; i < N; i++) {
        nze_serial[i] = 0;
        for (int j = 0; j < N; j++)
            nze_serial[i] += (a[i][j] != 0);
    }

    int *row_serial = malloc((N+1) * sizeof(int));
    if (row_serial == NULL) {
        printf("Memory allocation failed for row_serial\n");
        return 1;
    }
    row_serial[0] = 0;
    for (int i = 1; i <= N; i++)
        row_serial[i] = row_serial[i-1] + nze_serial[i-1];

    int nz = row_serial[N];
    int *v_serial = malloc(nz * sizeof(int));
    int *col_serial = malloc(nz * sizeof(int));
    if (v_serial == NULL || col_serial == NULL) {
        printf("Memory allocation failed for CSR arrays\n");
        return 1;
    }

    for (int i = 0; i < N; i++) {
        int pos = row_serial[i];
        for (int j = 0; j < N; j++)
            if (a[i][j] != 0) {
                v_serial[pos] = a[i][j];
                col_serial[pos++] = j;
            }
    }

    t1 = get_time();
    printf("Serial CSR formation time: %f seconds\n", t1-t0);

    //make the CRS representation using OpenMP
    t2 = get_time();

    int *nze_per_line = malloc(N * sizeof(int));
    #pragma omp parallel for
    for (int i = 0 ; i < N ; i++) {
        nze_per_line[i] = 0;
        for (int j = 0 ; j < N ; j++)
            nze_per_line[i] += (a[i][j] != 0);
    }

    int *row_index = malloc((N + 1) * sizeof(int));
    // Simple prefix sum - for better performance consider parallel prefix sum
    row_index[0] = 0;
    for (int i = 1; i <= N ; i++)
        row_index[i] = nze_per_line[i -1] + row_index[i-1];

    int non_zero_elements = row_index[N];
    int *v = malloc(non_zero_elements * sizeof(int));
    int *col_index = malloc(non_zero_elements * sizeof(int));

    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        int pos = row_index[i];
        for (int j = 0; j < N; j++)
            if (a[i][j] != 0) {
                v[pos] = a[i][j];
                col_index[pos++] = j;
            }
    }

    t3 = get_time();
    printf("Parallel CSR formation time: %f seconds\n", t3-t2);

    int *x_orig = malloc(N * sizeof(int));
    if (x_orig == NULL) {
        printf("Memory allocation failed for x_orig\n");
        return 1;
    }
    memcpy(x_orig, x, N*sizeof(int));
    
    // Arrays to store results for verification
    int *y_csr_serial = malloc(N * sizeof(int));
    int *y_csr_parallel = malloc(N * sizeof(int));
    int *y_dense_serial = malloc(N * sizeof(int));
    int *y_dense_parallel = malloc(N * sizeof(int));
    if (y_csr_serial == NULL || y_csr_parallel == NULL || 
        y_dense_serial == NULL || y_dense_parallel == NULL) {
        printf("Memory allocation failed for result arrays\n");
        return 1;
    }

    //check formation correctness - Compare serial and parallel CSR representations
    int formation_correct = 1;
    if (nz != non_zero_elements) {
        formation_correct = 0;
        printf("CSR Formation Error: Different number of non-zero elements\n");
    } else {
        // Check row indices
        for (int i = 0; i <= N; i++) {
            if (row_serial[i] != row_index[i]) {
                formation_correct = 0;
                printf("CSR Formation Error: row_index mismatch at position %d\n", i);
                break;
            }
        }
        // Check values and column indices
        if (formation_correct) {
            for (int i = 0; i < nz; i++) {
                if (v_serial[i] != v[i] || col_serial[i] != col_index[i]) {
                    formation_correct = 0;
                    printf("CSR Formation Error: Value or column index mismatch at position %d\n", i);
                    break;
                }
            }
        }
    }
    if (formation_correct) {
        printf("CSR Formation Verification: PASS\n");
    } else {
        printf("CSR Formation Verification: FAILED\n");
    }

    ///////////CSR multiplication/////////////

    //serial sparse matrix-vector multiplication of CSR formatted matrix
    memcpy(x, x_orig, N*sizeof(int));
    t4 = get_time();

    for (int it = 0; it < iterations; it++) {
        for (int i = 0; i < N; i++) {
            int sum = 0;
            for (int p = row_serial[i]; p < row_serial[i+1]; p++)
                sum += v_serial[p] * x[col_serial[p]];
            y_csr_serial[i] = sum;
        }
        // Swap x and y_csr_serial for next iteration
        int *tmp = x; x = y_csr_serial; y_csr_serial = tmp;
    }

    t5 = get_time();
    printf("Serial CSR multiplication time: %f seconds\n", t5-t4);

    //parallel sparse matrix-vector multiplication of CSR formatted matrix
    memcpy(x, x_orig, N*sizeof(int));
    t6 = get_time();

    for (int it = 0; it < iterations; it++) {
        #pragma omp parallel for
        for (int i = 0; i < N; i++) {
            int sum = 0;
            for (int p = row_index[i]; p < row_index[i+1]; p++)
                sum += v[p] * x[col_index[p]];
            y_csr_parallel[i] = sum;
        }
        int *tmp = x; x = y_csr_parallel; y_csr_parallel = tmp;
    }

    t7 = get_time();
    printf("Parallel CSR multiplication time: %f seconds\n", t7-t6);

    //check CSR multiplication correctness - Compare serial and parallel CSR multiplication results
    int csr_mul_correct = 1;
    for (int i = 0; i < N; i++) {
        if (y_csr_serial[i] != y_csr_parallel[i]) {
            csr_mul_correct = 0;
            printf("CSR Multiplication Error: Result mismatch at position %d\n", i);
            break;
        }
    }
    if (csr_mul_correct) {
        printf("CSR Multiplication Verification: PASS\n");
    } else {
        printf("CSR Multiplication Verification: FAILED\n");
    }

    ///////////dense multiplication/////////////

    //serial dense multiplication
    memcpy(x, x_orig, N*sizeof(int));
    t8 = get_time();

    for (int it = 0; it < iterations; it++) {
        for (int i = 0; i < N; i++) {
            int sum = 0;
            for (int j = 0; j < N; j++)
                sum += a[i][j] * x[j];
            y_dense_serial[i] = sum;
        }
        int *tmp = x; x = y_dense_serial; y_dense_serial = tmp;
    }

    t9 = get_time();
    printf("Serial dense multiplication time: %f seconds\n", t9-t8);

    //parallel dense multiplication
    memcpy(x, x_orig, N*sizeof(int));
    t10 = get_time();

    for (int it = 0; it < iterations; it++) {
        #pragma omp parallel for
        for (int i = 0; i < N; i++) {
            int sum = 0;
            for (int j = 0; j < N; j++)
                sum += a[i][j] * x[j];
            y_dense_parallel[i] = sum;
        }
        int *tmp = x; x = y_dense_parallel; y_dense_parallel = tmp;
    }

    t11 = get_time();
    printf("Parallel dense multiplication time: %f seconds\n", t11-t10);

    //check dense multiplication correctness - Compare serial and parallel dense multiplication results
    int dense_mul_correct = 1;
    for (int i = 0; i < N; i++) {
        if (y_dense_serial[i] != y_dense_parallel[i]) {
            dense_mul_correct = 0;
            printf("Dense Multiplication Error: Result mismatch at position %d\n", i);
            break;
        }
    }
    if (dense_mul_correct) {
        printf("Dense Multiplication Verification: PASS\n");
    } else {
        printf("Dense Multiplication Verification: FAILED\n");
    }

    // Final verification - Compare CSR and dense multiplication results
    // (Compare serial CSR with serial dense as they should be identical)
    int final_correct = 1;
    for (int i = 0; i < N; i++) {
        if (y_csr_serial[i] != y_dense_serial[i]) {
            final_correct = 0;
            printf("Final Verification Error: CSR and dense results differ at position %d\n", i);
            break;
        }
    }

    if (final_correct) {
        printf("Final Verification (CSR vs Dense): PASS\n");
    } else {
        printf("Final Verification (CSR vs Dense): FAILED\n");
    }

    //free allocated memory
    for (int i = 0 ; i < N ; i++) free(a[i]);
    free(a); free(x); free(x_orig); 
    free(y_csr_serial); free(y_csr_parallel); free(y_dense_serial); free(y_dense_parallel);
    free(nze_per_line); free(row_index); free(v); free(col_index);
    free(nze_serial); free(row_serial); free(v_serial); free(col_serial);

    return 0;
}