/*
 * Exercise 1.3: Array Analysis with False Sharing
 * Usage: ./array_stats <array_size>
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

// Struct definition that causes False Sharing (fields are adjacent in memory)
// struct array_stats_s {
//     long long int info_array_0;
//     long long int info_array_1;
//     long long int info_array_2;
//     long long int info_array_3;
// } array_stats;

// Improved struct definition to avoid False Sharing
struct array_stats_s {
    long long int info_array_0 __attribute__((aligned(64)));
    long long int info_array_1 __attribute__((aligned(64)));
    long long int info_array_2 __attribute__((aligned(64)));
    long long int info_array_3 __attribute__((aligned(64)));
} array_stats;

// Global variables for arrays and size
int *arrays[4];
int array_size;

// Helper function to get current time in seconds
double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}

// Thread function: Analyzes one array and updates shared struct directly
void *thread_func(void *arg) {
    long id = (long)arg;
    
    // Iterate through the assigned array
    for (int i = 0; i < array_size; i++) {
        if (arrays[id][i] != 0) {
            // Update to shared struct triggers False Sharing
            if (id == 0) array_stats.info_array_0++;
            else if (id == 1) array_stats.info_array_1++;
            else if (id == 2) array_stats.info_array_2++;
            else if (id == 3) array_stats.info_array_3++;
        }
    }
    return NULL;
}

// Serial version for performance comparison and verification
void serial_analysis(long long *results) {
    for (int k = 0; k < 4; k++) {
        results[k] = 0;
        for (int i = 0; i < array_size; i++) {
            if (arrays[k][i] != 0) {
                results[k]++;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <array_size>\n", argv[0]);
        return 1;
    }

    array_size = atoi(argv[1]);
    
    // Initialization
    double t_start = get_time();
    
    // Allocate memory and fill with random numbers (0-9)
    srand(time(NULL));
    for (int k = 0; k < 4; k++) {
        arrays[k] = malloc(array_size * sizeof(int));
        for (int i = 0; i < array_size; i++) {
            arrays[k][i] = rand() % 10; 
        }
    }

    // Reset struct fields
    array_stats.info_array_0 = 0;
    array_stats.info_array_1 = 0;
    array_stats.info_array_2 = 0;
    array_stats.info_array_3 = 0;
    
    double t_init = get_time() - t_start;

    // Serial Execution
    long long serial_results[4];
    t_start = get_time();
    serial_analysis(serial_results);
    double t_serial = get_time() - t_start;

    // Parallel Execution
    t_start = get_time();
    pthread_t threads[4];
    
    // Create 4 threads, one for each array
    for (long i = 0; i < 4; i++) {
        pthread_create(&threads[i], NULL, thread_func, (void*)i);
    }
    
    // Wait for threads to finish
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }
    double t_parallel = get_time() - t_start;

    // Verification
    int pass = 1;
    if (array_stats.info_array_0 != serial_results[0]) pass = 0;
    if (array_stats.info_array_1 != serial_results[1]) pass = 0;
    if (array_stats.info_array_2 != serial_results[2]) pass = 0;
    if (array_stats.info_array_3 != serial_results[3]) pass = 0;

    // Output CSV format: Size, InitTime, SerialTime, ParallelTime, Verification
    printf("%d,%.6f,%.6f,%.6f,%s\n", array_size, t_init, t_serial, t_parallel, pass ? "PASS" : "FAIL");

    // Cleanup memory
    for (int k=0; k<4; k++)
        free(arrays[k]);

    return 0;
}