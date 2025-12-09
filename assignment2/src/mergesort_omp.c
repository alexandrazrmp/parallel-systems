/*
 * assignment2/src/mergesort_omp.c
 * Exercise 2.3 - Mergesort using OpenMP Tasks
 * Usage: ./bin/mergesort_omp <size> <algo_type> <num_threads>
 * algo_type: 0 for Serial, 1 for Parallel
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <sys/time.h>

// Threshold to switch from parallel tasks to serial execution
// Creating tasks for very small arrays adds unnecessary overhead.
#define TASK_THRESHOLD 10000

// Helper function: Merges two sorted subarrays arr[left..mid] and arr[mid+1..right]
void merge(int *arr, int *temp, int left, int mid, int right) {
    int i = left;
    int j = mid + 1;
    int k = left;

    // Copy data to temp array while merging
    while (i <= mid && j <= right) {
        if (arr[i] <= arr[j]) {
            temp[k++] = arr[i++];
        } else {
            temp[k++] = arr[j++];
        }
    }

    // Copy remaining elements
    while (i <= mid) {
        temp[k++] = arr[i++];
    }
    while (j <= right) {
        temp[k++] = arr[j++];
    }

    // Copy back to original array
    for (i = left; i <= right; i++) {
        arr[i] = temp[i];
    }
}

// Serial Mergesort (standard recursion)
void mergesort_serial(int *arr, int *temp, int left, int right) {
    if (left < right) {
        int mid = (left + right) / 2;
        mergesort_serial(arr, temp, left, mid);
        mergesort_serial(arr, temp, mid + 1, right);
        merge(arr, temp, left, mid, right);
    }
}

// Parallel Mergesort using OpenMP Tasks
void mergesort_parallel(int *arr, int *temp, int left, int right) {
    if (left < right) {
        int mid = (left + right) / 2;
        
        // Calculate size of current chunk
        int size = right - left + 1;

        // Use 'task' directive.
        // The 'if' clause prevents creating overhead for small arrays.
        // If size < TASK_THRESHOLD, the task is executed immediately by the current thread (serially).
        
        #pragma omp task if(size > TASK_THRESHOLD) shared(arr, temp)
        mergesort_parallel(arr, temp, left, mid);

        #pragma omp task if(size > TASK_THRESHOLD) shared(arr, temp)
        mergesort_parallel(arr, temp, mid + 1, right);

        // Wait for both child tasks to finish before merging
        #pragma omp taskwait
        
        merge(arr, temp, left, mid, right);
    }
}

double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <array_size> <algo: 0=Serial, 1=Parallel> <num_threads>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    int algo = atoi(argv[2]); // 0 = Serial, 1 = Parallel
    int threads = atoi(argv[3]);

    if (n <= 0) return 1;

    // Memory Allocation
    int *arr = (int *)malloc(n * sizeof(int));
    int *temp = (int *)malloc(n * sizeof(int)); // Temp array needed for merge step

    // Initialize with deterministic random numbers
    srand(42);
    for (int i = 0; i < n; i++) {
        arr[i] = rand();
    }

    printf("Sorting array of size %d using %s algorithm with %d threads...\n", 
           n, (algo == 1 ? "Parallel" : "Serial"), threads);

    // Set number of threads
    omp_set_num_threads(threads);

    double start = get_time();

    if (algo == 0) {
        // Serial execution
        mergesort_serial(arr, temp, 0, n - 1);
    } else {
        // Parallel execution
        // We need a 'parallel' region to spawn threads.
        // 'single' ensures the root call is made by only one thread.
        #pragma omp parallel
        {
            #pragma omp single
            {
                mergesort_parallel(arr, temp, 0, n - 1);
            }
        }
    }

    double end = get_time();
    printf("Execution time: %.6f seconds\n", end - start);

    // Verification
    int valid = 1;
    for (int i = 0; i < n - 1; i++) {
        if (arr[i] > arr[i + 1]) {
            valid = 0;
            break;
        }
    }
    printf("Verification: %s\n", valid ? "PASS" : "FAIL");

    free(arr);
    free(temp);
    return 0;
}