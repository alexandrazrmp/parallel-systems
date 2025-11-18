/*
 * assignment1/src/array_stats_improved.c
 * Exercise 1.2 - array statistics
 * Usage: ./bin/array_stats_improved <array_size> [num_threads]
 *
 * Simple implementation of counting non-zero elements in arrays using both serial and parallel approaches.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

// Struct to hold the counts for each array
struct array_stats_s {
    long long int info_array_0;
    long long int info_array_1;
    long long int info_array_2;
    long long int info_array_3;
} array_stats;

// One mutex per counter to protect updates.
static pthread_mutex_t counter_mutex[4];

// Structure for thread arguments
typedef struct {
    int thread_id;
    int num_threads;            // total number of threads
    int **arrays;               // pointers to arrays
    int n;                      // length of the arrays
    long long int **counters;   // pointers to the shared counters for each array
    pthread_mutex_t *mutexes;   // pointer to the array of mutexes (one per array)
} ThreadArgs;

// Return current time in seconds
static double get_time(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}

// Thread routine: count non-zero in provided array
static void* count_non_zero_parallel(void* arg) {
    ThreadArgs* args = (ThreadArgs*) arg;
    int tid = args->thread_id;
    int tcount = args->num_threads;
    int len = args->n;

    // For each of the 4 arrays, simple round-robin by array index
    for (int a = 0; a < 4; a++) {
        if ((a % tcount) != tid) continue;  // not this thread's array

        int *arr = args->arrays[a];
        long long int local_count = 0;
        for (int i = 0; i < len; i++) {
            if (arr[i] != 0) local_count++;
        }

        // update shared counter for this array with locking
        pthread_mutex_lock(&args->mutexes[a]);
        *args->counters[a] += local_count;
        pthread_mutex_unlock(&args->mutexes[a]);
    }

    return NULL;
}

// Serial routine: count non-zero in all arrays
static void count_non_zero_serial(int len, int *a0, int *a1, int *a2, int *a3) {
     long long int c0 = 0, c1 = 0, c2 = 0, c3 = 0;

     for (int i = 0; i < len; i++) if (a0[i] != 0) c0++;
     for (int i = 0; i < len; i++) if (a1[i] != 0) c1++;
     for (int i = 0; i < len; i++) if (a2[i] != 0) c2++;
     for (int i = 0; i < len; i++) if (a3[i] != 0) c3++;

    // Write serial results into global struct.
    array_stats.info_array_0 = c0;
    array_stats.info_array_1 = c1;
    array_stats.info_array_2 = c2;
    array_stats.info_array_3 = c3;
}

int main(int argc, char* argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <array_size> [num_threads]\n", argv[0]);
        return EXIT_FAILURE;
    }

    int n = atoi(argv[1]);
    if (n <= 0) {
        fprintf(stderr, "Array size must be a positive integer\n");
        return EXIT_FAILURE;
    }

    int num_threads = 4;
    if (argc == 3) {
        num_threads = atoi(argv[2]);
        if (num_threads <= 0) {
            fprintf(stderr, "Number of threads must be a positive integer\n");
            return EXIT_FAILURE;
        }
    }

    double start_time, end_time;
    long long int s0, s1, s2, s3;   // local copies for serial results
    long long int p0, p1, p2, p3;   // local copies for parallel results

    // Initialization
    start_time = get_time();

    int *array0 = malloc((size_t)n * sizeof(int));
    int *array1 = malloc((size_t)n * sizeof(int));
    int *array2 = malloc((size_t)n * sizeof(int));
    int *array3 = malloc((size_t)n * sizeof(int));
    if (!array0 || !array1 || !array2 || !array3) {
        fprintf(stderr, "Memory allocation failed\n");
        return EXIT_FAILURE;
    }

    srand(42); // reproducible
    for (int i = 0; i < n; i++) {
        array0[i] = rand() % 10;
        array1[i] = rand() % 10;
        array2[i] = rand() % 10;
        array3[i] = rand() % 10;
    }

    // initialize mutexes
    for (int i = 0; i < 4; i++)
        pthread_mutex_init(&counter_mutex[i], NULL);

    array_stats.info_array_0 = 0;
    array_stats.info_array_1 = 0;
    array_stats.info_array_2 = 0;
    array_stats.info_array_3 = 0;

    end_time = get_time();
    printf("Initialization time: %.6f seconds\n", end_time - start_time);

    // Serial computation
    start_time = get_time();
    count_non_zero_serial(n, array0, array1, array2, array3);

    // read serial results
    s0 = array_stats.info_array_0;
    s1 = array_stats.info_array_1;
    s2 = array_stats.info_array_2;
    s3 = array_stats.info_array_3;
    end_time = get_time();
    printf("Serial execution time: %.6f seconds\n", end_time - start_time);

    // Parallel computation
    start_time = get_time();

    // reset counters
    array_stats.info_array_0 = 0;
    array_stats.info_array_1 = 0;
    array_stats.info_array_2 = 0;
    array_stats.info_array_3 = 0;

    // prepare arrays and counters for flexible thread counts
    int *arrays[4] = { array0, array1, array2, array3 };
    long long int *counters[4] = { &array_stats.info_array_0, &array_stats.info_array_1, &array_stats.info_array_2, &array_stats.info_array_3 };

    pthread_t threads[num_threads];
    ThreadArgs args[num_threads];

    for (int t = 0; t < num_threads; t++) {
        args[t].thread_id = t;
        args[t].num_threads = num_threads;
        args[t].arrays = &arrays[0];
        args[t].n = n;
        args[t].counters = &counters[0];
        args[t].mutexes = &counter_mutex[0];

        int create_status = pthread_create(&threads[t], NULL, count_non_zero_parallel, &args[t]);
        if (create_status != 0) {
            fprintf(stderr, "Error creating thread %d\n", t);
            return EXIT_FAILURE;
        }
    }

    // wait for threads to finish
    for (int t = 0; t < num_threads; t++) {
        pthread_join(threads[t], NULL);
    }


    // read parallel results
    p0 = array_stats.info_array_0;
    p1 = array_stats.info_array_1;
    p2 = array_stats.info_array_2;
    p3 = array_stats.info_array_3;

    end_time = get_time();
    printf("Parallel execution time: %.6f seconds\n", end_time - start_time);

    // destroy mutexes
    for (int i = 0; i < 4; i++)
        pthread_mutex_destroy(&counter_mutex[i]);

    // Verification
    int valid = 1;
    if (s0 != p0 || s1 != p1 || s2 != p2 || s3 != p3) valid = 0;
    printf("Verification: %s\n", valid ? "PASS" : "FAIL");

    // Print results
    printf("\nResults:\n");
    printf("Array 0 - Serial: %lld, Parallel: %lld\n", s0, p0);
    printf("Array 1 - Serial: %lld, Parallel: %lld\n", s1, p1);
    printf("Array 2 - Serial: %lld, Parallel: %lld\n", s2, p2);
    printf("Array 3 - Serial: %lld, Parallel: %lld\n", s3, p3);

    // Free allocated memory
    free(array0);
    free(array1);
    free(array2);
    free(array3);

    return 0;
}