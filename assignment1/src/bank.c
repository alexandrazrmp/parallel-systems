/*
 * assignment1/src/bank.c
 * Exercise 1.4 - Bank Simulation
 * Usage: ./bin/shared_var_update <iteration_times> <num_threads>
 *
 * bank simulation with transactions on an "account balance" array using parallel programming
 * you can comment out this bit
                for (volatile int i = 0; i < 1000; i++) {
                    // tiny busy-wait loop
                }
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/time.h>
#include <unistd.h>

#define money_transfer 1
#define balance_check 2

#define course_grained_mutex 1
#define fine_grained_mutex 2
#define course_grained_rw_lock 3
#define fine_grained_rw_lock 4

//locks for each approach
pthread_mutex_t global_mutex;
pthread_mutex_t *mutexes = NULL;
pthread_rwlock_t global_rwlock;
pthread_rwlock_t *rwlocks = NULL;

int n_elements;
int transactions_per_thread;
int query_percentage;
int lock_type;
int num_threads;
int *balances;

// Thread function performing transactions
void *thread_func(void *arg) {
    unsigned int seed = (uintptr_t)arg ^ time(NULL); // per-thread seed for rand_r  (thread-safe)
    for (int t = 0; t < transactions_per_thread; t++) {
        int op = (rand_r(&seed) % 100 < query_percentage) ? balance_check : money_transfer;     //deciding on transaction operation

        if (op == balance_check) {
            int idx = rand_r(&seed) % n_elements;
            // Lock depending on the approach
            if (lock_type == course_grained_mutex) {
                pthread_mutex_lock(&global_mutex);
                 for (volatile int i = 0; i < 1000; i++) {
                     // tiny busy-wait loop
                 }
                pthread_mutex_unlock(&global_mutex);
            } else if (lock_type == fine_grained_mutex) {
                pthread_mutex_lock(&mutexes[idx]);
                 for (volatile int i = 0; i < 1000; i++) {
                     // tiny busy-wait loop
                 }
                pthread_mutex_unlock(&mutexes[idx]);
            } else if (lock_type == course_grained_rw_lock) {
                pthread_rwlock_rdlock(&global_rwlock);
                 for (volatile int i = 0; i < 1000; i++) {
                     // tiny busy-wait loop
                 }
                pthread_rwlock_unlock(&global_rwlock);
            } else if (lock_type == fine_grained_rw_lock) {
                pthread_rwlock_rdlock(&rwlocks[idx]);
                 for (volatile int i = 0; i < 1000; i++) {
                     // tiny busy-wait loop
                 }

                pthread_rwlock_unlock(&rwlocks[idx]);
            }
        } else { // money_transfer
            int from = rand_r(&seed) % n_elements;
            int to = rand_r(&seed) % n_elements;
            while (to == from) to = rand_r(&seed) % n_elements;
            int amount = rand_r(&seed) % 100;

            if (lock_type == course_grained_mutex) {
                pthread_mutex_lock(&global_mutex);
                if (balances[from] >= amount) {
                    balances[from] -= amount;
                    balances[to] += amount;
                }

                pthread_mutex_unlock(&global_mutex);
            } else if (lock_type == fine_grained_mutex) {
                int first = from < to ? from : to;
                int second = from < to ? to : from;
                pthread_mutex_lock(&mutexes[first]);
                pthread_mutex_lock(&mutexes[second]);
                if (balances[from] >= amount) {
                    balances[from] -= amount;
                    balances[to] += amount;
                }

                pthread_mutex_unlock(&mutexes[second]);
                pthread_mutex_unlock(&mutexes[first]);
            } else if (lock_type == course_grained_rw_lock) {
                pthread_rwlock_wrlock(&global_rwlock);
                if (balances[from] >= amount) {
                    balances[from] -= amount;
                    balances[to] += amount;
                }

                pthread_rwlock_unlock(&global_rwlock);
            } else if (lock_type == fine_grained_rw_lock) {
                int first = from < to ? from : to;
                int second = from < to ? to : from;
                pthread_rwlock_wrlock(&rwlocks[first]);
                pthread_rwlock_wrlock(&rwlocks[second]);
                if (balances[from] >= amount) {
                    balances[from] -= amount;
                    balances[to] += amount;
                }

                pthread_rwlock_unlock(&rwlocks[second]);
                pthread_rwlock_unlock(&rwlocks[first]);
            }
        }
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
    if (argc != 6) {
        fprintf(stderr, "Usage: %s <n_elements> <transactions_per_thread> <query_percentage> <lock_type> <num_threads>\n", argv[0]);
        return 1;
    }

    n_elements = atoi(argv[1]);
    transactions_per_thread = atoi(argv[2]);
    query_percentage = atoi(argv[3]);
    lock_type = atoi(argv[4]);
    num_threads = atoi(argv[5]);

    balances = malloc(n_elements * sizeof(int));
    if (!balances) {
        perror("malloc");
        return 1;
    }

    //lock type initialization
    if (lock_type == course_grained_mutex) {
        pthread_mutex_init(&global_mutex, NULL);
    } else if (lock_type == fine_grained_mutex) {
        mutexes = malloc(n_elements * sizeof(pthread_mutex_t));
        if (!mutexes) { perror("malloc"); return 1; }
        for (int i = 0; i < n_elements; i++) pthread_mutex_init(&mutexes[i], NULL);
    } else if (lock_type == course_grained_rw_lock) {
        pthread_rwlock_init(&global_rwlock, NULL);
    } else if (lock_type == fine_grained_rw_lock) {
        rwlocks = malloc(n_elements * sizeof(pthread_rwlock_t));
        if (!rwlocks) { perror("malloc"); return 1; }
        for (int i = 0; i < n_elements; i++) pthread_rwlock_init(&rwlocks[i], NULL);
    } else {
        fprintf(stderr, "Unknown lock type %d\n", lock_type);
        return 1;
    }

    int initial_amount_of_money = 0;    //to check correctness

    //shared array initialization
    srand(time(NULL));
    for (int i = 0 ; i < n_elements ; i++){
        balances[i] = rand() % 1000;
        initial_amount_of_money += balances[i];
    }

    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    int *thread_args = malloc(num_threads * sizeof(int));
    if (!threads || !thread_args) { perror("malloc"); return 1; }

    double start_time = get_time();
    for (int i = 0; i < num_threads; i++) {
        thread_args[i] = i;
        if (pthread_create(&threads[i], NULL, thread_func, (void*)(uintptr_t)thread_args[i]) != 0) {
            fprintf(stderr, "Error creating thread\n");
            return EXIT_FAILURE;
        }
    }
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    double end_time = get_time();
    double time = end_time - start_time;
    printf("Execution time: %.6f seconds\n", time);

    int final_amount_of_money = 0;    //to check correctness
    for (int i = 0 ; i < n_elements ; i++){
        final_amount_of_money += balances[i];
    }

    //checking program correctness
    if (initial_amount_of_money != final_amount_of_money) printf("Transactions failed\n");
    else printf("Verification: PASS\n");

    // destroy locks
    if (lock_type == course_grained_mutex) {
        pthread_mutex_destroy(&global_mutex);
    } else if (lock_type == fine_grained_mutex) {
        for (int i = 0; i < n_elements; i++) pthread_mutex_destroy(&mutexes[i]);
        free(mutexes);
    } else if (lock_type == course_grained_rw_lock) {
        pthread_rwlock_destroy(&global_rwlock);
    } else if (lock_type == fine_grained_rw_lock) {
        for (int i = 0; i < n_elements; i++) pthread_rwlock_destroy(&rwlocks[i]);
        free(rwlocks);
    }

    free(threads);
    free(thread_args);
    free(balances);
    return(0);
}
