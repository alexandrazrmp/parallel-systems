/*
 * assignment2/src/sparse_matrix_vector_mult.c
 * Exercise 2.2 - Sparce Matrix-Vector Multiplication
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
    for (int i = 0 ; i < N ; i++) a[i] = malloc(N*sizeof(int)); 
    int *x;
    x = malloc(N*sizeof(int));

    //fill in random matrix according to sparsity percentage
    //also fill in vector x
    for (int i = 0; i < N; i++) {
        x[i] = rand() % 1000;
        for (int j = 0; j < N; j++) {
            int r = rand() % 100;
            if (r < sparsity)
                a[i][j] = 0;
            else
                a[i][j] = rand() % 1000 + 1;   // non-zero from 1..100
        }
    }

    printf("N = %d\n", N);
    printf("Sparsity = %d%%\n", sparsity);
    printf("Iterations = %d\n", iterations);
    printf("Threads = %d\n", threads);


    double t1, t2;    // Timing variables
    t1 = get_time();

    //make the CRS representation using OpenMP

    omp_set_num_threads(threads);

    //non zero elements per line
    int *nze_per_line = malloc(N * sizeof(int));
    #pragma omp parallel for num_threads(threads)
    for (int i = 0 ; i < N ; i++) {     //each thread has a row
        nze_per_line[i] = 0;
        for (int j = 0 ; j < N ; j++) {
            nze_per_line[i] += (a[i][j] != 0);
        }
    }

    int *row_index = malloc((N + 1) * sizeof(int));

    //row index array must be filled serially
    row_index[0] = 0;
    for (int i = 1; i <= N ; i++)
        row_index[i] = nze_per_line[i -1] + row_index[i-1]; 

    //total non zero elements
    int non_zero_elements = row_index[N];

    int *v = malloc(non_zero_elements * sizeof(int));
    int *col_index = malloc(non_zero_elements * sizeof(int));

    //fill value and column arrays
    #pragma omp parallel for num_threads(threads)
    for (int i = 0; i < N; i++) {   //each thread has each row
        int pos = row_index[i];   //starting write position for this row
        for (int j = 0; j < N; j++) {
            if (a[i][j] != 0) {
                v[pos] = a[i][j];
                col_index[pos] = j;
                pos++;
            }
        }
    }


    t2 = get_time();
    printf("CRS formation time: %f seconds\n", t2-t1);


    //save original x for dense multiplication
    int *x_orig = malloc(N * sizeof(int));
    memcpy(x_orig, x, N*sizeof(int));

    //sparce multiplication
    int *y = malloc(N * sizeof(int));

    double t3 = get_time();

    for (int iter = 0; iter < iterations; iter++) {
        #pragma omp parallel for
        for (int i = 0; i < N; i++) {
            int sum = 0;
            for (int p = row_index[i]; p < row_index[i+1]; p++)
                sum += v[p] * x[col_index[p]];
            y[i] = sum;
        }

        // y becomes next iteration's input (in-place swap)
        int *tmp = x;
        x = y;
        y = tmp;
    }

    double t4 = get_time();
    printf("CSR multiplication time: %f seconds\n", t4 - t3);
    printf("TOTAL CSR time: %f seconds\n", t4 - t3 + t2 - t1);


    //reset x
    memcpy(x, x_orig, N*sizeof(int));

    //dense multiplication

    int *z = malloc(N * sizeof(int));

    double t5 = get_time();

    for (int iter = 0; iter < iterations; iter++) {
        for (int i = 0; i < N; i++) {
            int sum = 0;
            for (int j = 0; j < N; j++)
                sum += a[i][j] * x[j];
            z[i] = sum;
        }

        int *tmp = x;
        x = z;
        z = tmp;
    }

    double t6 = get_time();
    printf("Dense multiplication time: %f seconds\n", t6 - t5);


    //checking program correctness
    int correct = 1;

    for (int i = 0; i < N; i++) {
        if (y[i] != z[i]) { //compare last iteration results
            correct = 0;
            break;
        }
    }

    if (correct) 
        printf("Verification: PASS\n");
    else 
        printf("Verification: FAILED\n");


    //free allocated memory
    for (int i = 0 ; i < N ; i++) free(a[i]);
    free(a);
    free(x);
    free(x_orig);
    free(y);
    free(z);
    free(nze_per_line);
    free(row_index);
    free(v);
    free(col_index);

    return 0;
}
