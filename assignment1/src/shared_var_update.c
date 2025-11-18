/*
 * assignment1/src/shared_var_update.c
 * Exercise 1.2 - shared variable update
 * Usage: ./bin/shared_var_update <iteration_times> <num_threads>
 *
 * Shared variable updates using both serial and parallel approaches with pthreads (mutex, read-write lock, atomic operations).
*/


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/time.h>

//shared variables for different approaches
int shared_var_mutex;
int shared_var_rwlock;
int shared_var_atomic;

int iteration_times;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;


//thread function for mutex approach
void* thread_func_mutex(void* arg) {
    for (int i = 0; i < iteration_times; i++) {
        //update shared variable with mutex
        pthread_mutex_lock(&mutex);
        shared_var_mutex++;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
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
        fprintf(stderr, "Usage: %s <iteration_times> <num_threads>\n", argv[0]);
    }
    iteration_times = atoi(argv[1]);
    int num_threads = atoi(argv[2]);
    
    int shared_var = 0;

    //serial approach
    for (int i = 0; i < num_threads; i++) {
        for (int j = 0; j < iteration_times; j++) {
            shared_var++; //serial update
        }
    }
    printf("Serial approach - Final value of shared_var: %d\n", shared_var);



    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    if (!threads) {
        fprintf(stderr, "Thread allocation failed\n");
        exit(EXIT_FAILURE);
    }

    double start_time, end_time;    // Timing variables

    //mutex approach
    start_time = get_time();
    for (int i = 0; i < num_threads; i++) {
        //create threads for mutex approach
        if (pthread_create(&threads[i], NULL, thread_func_mutex, NULL) != 0) {
            fprintf(stderr, "Error creating thread\n");
            return EXIT_FAILURE;
        }
    }
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    end_time = get_time();
    printf("Mutex approach - Final value of shared_var: %d\n", shared_var);
    printf("Mutex approach time: %.6f seconds\n", end_time - start_time);


    //read-write lock





    //atomic operations


}