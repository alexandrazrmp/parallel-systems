/*
 * assignment2/src/poly_mult_omp.c
 * Exercise 2.1 - polynomial multiplication with OpenMP
 * Usage: ./bin/poly_mult_omp <degree_n> <num_threads>
 *
 * Implementation of polynomial multiplication using OpenMP.
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>        // OpenMP header
#include <stdint.h>
#include <sys/time.h>

// Function for serial polynomial multiplication (for verification)
void multiply_polynomials_serial(int64_t *A, int64_t *B, int64_t *C_serial, int n) {
    for (int i = 0; i <= 2 * n; i++) {
        int64_t sum = 0;
        int jmin = (i - n) > 0 ? (i - n) : 0;
        int jmax = i < n ? i : n;
        for (int j = jmin; j <= jmax; j++) {
            sum += A[j] * B[i - j];
        }
        C_serial[i] = sum;
    }
}

// Function to get the current time in seconds
double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}

int main(int argc, char* argv[]) {
    // Argument parsing
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <polynomial_degree> <num_threads>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int n, num_threads;
    int64_t *A, *B, *C_serial, *C_parallel;

    n = atoi(argv[1]);
    num_threads = atoi(argv[2]);
    if (n <= 0 || num_threads <= 0) {
        fprintf(stderr, "Arguments must be positive integers\n");
        exit(EXIT_FAILURE);
    }

    // Set the number of threads for OpenMP
    omp_set_num_threads(num_threads);

    double start_time, end_time;    // Timing variables

    // Allocate and initialize polynomials
    start_time = get_time();
    A = (int64_t*)malloc((n + 1) * sizeof(int64_t));
    B = (int64_t*)malloc((n + 1) * sizeof(int64_t));
    C_serial = (int64_t*)malloc((2 * n + 1) * sizeof(int64_t));
    C_parallel = (int64_t*)malloc((2 * n + 1) * sizeof(int64_t));

    if (!A || !B || !C_serial || !C_parallel) {
        fprintf(stderr, "Allocation failed\n");
        exit(EXIT_FAILURE);
    }

    srand(42);  // For reproducibility
    for (int i = 0; i <= n; i++) {
        A[i] = (int64_t)((rand() % 20) - 10); // Random integer -10 to 10
        B[i] = (int64_t)((rand() % 20) - 10);
        if (A[i] == 0) A[i] = 1;
        if (B[i] == 0) B[i] = 1;
    }
    end_time = get_time();
    printf("Initialization time: %.6f seconds\n", end_time - start_time);

    // Serial polynomial multiplication
    start_time = get_time();
    multiply_polynomials_serial(A, B, C_serial, n);
    end_time = get_time();
    printf("Serial multiplication time: %.6f seconds\n", end_time - start_time);

    // Parallel polynomial multiplication (OpenMP)
    start_time = get_time();

    /* * OpenMP Parallel Loop
     * - shared(A, B, C_parallel, n): Arrays and n are read/written by all
     * - private(i, j, sum, jmin, jmax): Loop variables must be private to each thread
     * - schedule(dynamic): Used because workload varies (middle indices have more work)
     */
    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i <= 2 * n; i++) {
        int64_t sum = 0;
        int jmin = (i - n) > 0 ? (i - n) : 0;
        int jmax = i < n ? i : n;
        
        for (int j = jmin; j <= jmax; j++) {
            sum += A[j] * B[i - j];
        }
        C_parallel[i] = sum;
    }

    end_time = get_time();
    printf("Parallel multiplication time: %.6f seconds\n", end_time - start_time);

    // Verification of results
    int valid = 1;
    for (int i = 0; i <= 2 * n; i++) {
        if (C_serial[i] != C_parallel[i]) {
            valid = 0;
            break;
        }
    }
    printf("Verification: %s\n", valid ? "PASS" : "FAIL");

    // Free allocated memory
    free(A);
    free(B);
    free(C_serial);
    free(C_parallel);

    return 0;
}