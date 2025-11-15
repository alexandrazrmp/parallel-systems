/*
 * assignment1/src/poly_mult.c
 * Exercise 1.1 - polynomial multiplication
 * Usage: ./bin/poly_mult <degree_n> <num_threads>
 *
 * Simple implementation of polynomial multiplication using both serial and parallel approaches.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/time.h>

// Structure to hold arguments for each thread
typedef struct {
    int start;
    int end;
    int n;
    int64_t *A;
    int64_t *B;
    int64_t *R;
} ThreadArgs;

// Function executed by each thread for parallel polynomial multiplication
void* multiply_polynomials_parallel(void* arg) {
    ThreadArgs* args = (ThreadArgs*) arg;
    int n = args->n;
    int64_t *A = args->A;
    int64_t *B = args->B;
    int64_t *R = args->R;

    for (int i = args->start; i <= args->end; i++) {
        int64_t sum = 0;
        int jmin = (i - n) > 0 ? (i - n) : 0;
        int jmax = i < n ? i : n;
        for (int j = jmin; j <= jmax; j++) {
            sum += A[j] * B[i - j];
        }
        R[i] = sum;
    }
    return NULL;
}

// Function for serial polynomial multiplication
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

    // Parallel polynomial multiplication
    start_time = get_time();
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    ThreadArgs *thread_args = malloc(num_threads * sizeof(ThreadArgs));
    if (!threads || !thread_args) {
        fprintf(stderr, "Thread allocation failed\n");
        exit(EXIT_FAILURE);
    }

    int chunk_size = (2 * n + 1) / num_threads; // Calculate chunk size for each thread

    for (int i = 0; i < num_threads; i++) {
        thread_args[i].start = i * chunk_size;  // Start index for this thread
        thread_args[i].end = (i == num_threads - 1) ? (2 * n) : (thread_args[i].start + chunk_size - 1);  // End index for this thread
        thread_args[i].n = n;
        thread_args[i].A = A;
        thread_args[i].B = B;
        thread_args[i].R = C_parallel;
        pthread_create(&threads[i], NULL, multiply_polynomials_parallel, &thread_args[i]);
    }

    // Wait for all threads to finish
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
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
    free(threads);
    free(thread_args);

    return 0;
}
