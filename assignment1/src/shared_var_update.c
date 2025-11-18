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

//thread function for read-write lock approach
void* thread_func_rwlock(void* arg) {
    for (int i = 0; i < iteration_times; i++) {
        //update shared variable with read-write lock
        pthread_rwlock_wrlock(&rwlock);
        shared_var_rwlock++;
        pthread_rwlock_unlock(&rwlock);
    }
    return NULL;
}

//thread function for atomic approach
void* thread_func_atomic(void* arg) {
    for (int i = 0; i < iteration_times; i++) {
        //update shared variable with atomic operation
        __atomic_fetch_add(&shared_var_atomic, 1, __ATOMIC_SEQ_CST);
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


    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    if (!threads) {
        fprintf(stderr, "Thread allocation failed\n");
        exit(EXIT_FAILURE);
    }

    double start_time, end_time;    // Timing variables

    //mutex approach
    shared_var_mutex = 0;
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
    double mutex_time = end_time - start_time;
    printf("Mutex approach time: %.6f seconds\n", mutex_time);


    //read-write lock
    shared_var_rwlock = 0;
    start_time = get_time();
    for (int i = 0; i < num_threads; i++) {
        //create threads for rwlock approach
        if (pthread_create(&threads[i], NULL, thread_func_rwlock, NULL) != 0) {
            fprintf(stderr, "Error creating thread\n");
            return EXIT_FAILURE;
        }
    }
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    end_time = get_time();
    double rwlock_time = end_time - start_time;
    printf("Read-Write Lock approach time: %.6f seconds\n", rwlock_time);
    

    //atomic operations

    shared_var_atomic = 0;
    start_time = get_time();
    for (int i = 0; i < num_threads; i++) {
        //create threads for atomic approach
        if (pthread_create(&threads[i], NULL, thread_func_atomic, NULL) != 0) {
            fprintf(stderr, "Error creating thread\n");
            return EXIT_FAILURE; 
        }
    }
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    end_time = get_time();
    double atomic_time = end_time - start_time;
    printf("Atomic approach time: %.6f seconds\n", atomic_time);

    //verify results
    
    if (shared_var_mutex != shared_var) {
        printf("Mutex approach - Incorrect final value: %d\n", shared_var_mutex);
    }
    if (shared_var_rwlock != shared_var) {
        printf("Read-Write Lock approach - Incorrect final value: %d\n", shared_var_rwlock);
    }
    if (shared_var_atomic != shared_var) {
        printf("Atomic approach - Incorrect final value: %d\n", shared_var_atomic);
    }
    if (shared_var_mutex == shared_var && shared_var_rwlock == shared_var && shared_var_atomic == shared_var) {
        printf("Verification: PASS\n");
    }


    // Cleanup
    free(threads);
    pthread_mutex_destroy(&mutex);
    pthread_rwlock_destroy(&rwlock);


    return 0;

}