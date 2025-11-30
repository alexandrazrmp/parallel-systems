/*
 * assignment1/src/barriers/barrier_spin.c
 * Exercise 1.5 - spin barrier
 * Usage: ./bin/barrier_spin <num_threads> <iterations>
 *
 * Implementation of a sense-reversal spin barrier using pthreads.
 * Logic from: https://en.wikipedia.org/wiki/Barrier_(computer_science)
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdbool.h>

static double get_time(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}

typedef struct {
    int thread_barrier_number;  // Number of threads required to reach the barrier
    int total_thread;   // How many threads have reached the barrier
    pthread_mutex_t lock;   // Mutex to protect barrier state
    volatile bool flag; // use volatile to prevent compiler optimizations
} spin_barrier_t;

static void spin_barrier_init(spin_barrier_t *barrier, int thread_barrier_number) {
    pthread_mutex_init(&(barrier->lock), NULL);
    barrier->total_thread = 0;
    barrier->thread_barrier_number = thread_barrier_number;
    barrier->flag = false;
}

static void spin_barrier_destroy(spin_barrier_t *barrier) {
    pthread_mutex_destroy(&(barrier->lock));
}

static void spin_barrier_wait(spin_barrier_t *barrier) {
    bool local_sense = barrier->flag;   // read current flag value

    pthread_mutex_lock(&(barrier->lock));   // lock to update barrier state
    barrier->total_thread++;    // increment count of threads that have reached the barrier
    local_sense = !local_sense; // toggle local sense

    if (barrier->total_thread == barrier->thread_barrier_number) {  // last thread to arrive
        barrier->total_thread = 0;
        barrier->flag = local_sense;
        pthread_mutex_unlock(&(barrier->lock));
    } else {    // not the last thread
        pthread_mutex_unlock(&(barrier->lock));
        while (barrier->flag != local_sense);   // spin-wait until flag changes
    }
}

static int num_threads = 4;   // default number of threads
static int iterations = 1000000;  // default number of iterations

static spin_barrier_t start_barrier;
static spin_barrier_t measured_barrier;
static spin_barrier_t end_barrier;

void* thread_function(void* arg) {
    (void)arg; // thread id not used

    spin_barrier_wait(&start_barrier);  // wait until main and all threads are ready

    for (int i = 0; i < iterations; i++) {  // threads synchronize among themselves
        spin_barrier_wait(&measured_barrier);
    }

    spin_barrier_wait(&end_barrier);    // signal completion
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <num_threads> <iterations>\n", argv[0]);
        return EXIT_FAILURE;
    }

    num_threads = atoi(argv[1]);
    iterations = atoi(argv[2]);
    if (num_threads <= 0 || iterations <= 0) {
        fprintf(stderr, "Arguments must be positive integers\n");
        return EXIT_FAILURE;
    }

    pthread_t *threads = malloc(sizeof(pthread_t) * num_threads);
    if (!threads) {
        fprintf(stderr, "Allocation failed\n");
        return EXIT_FAILURE;
    }

    double t_init_start = get_time();

    // Initialize barriers
    spin_barrier_init(&start_barrier, num_threads + 1);
    spin_barrier_init(&measured_barrier, num_threads);
    spin_barrier_init(&end_barrier, num_threads + 1);

    // Create threads
    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&threads[i], NULL, thread_function, NULL) != 0) {
            perror("Failed to create thread");
            return EXIT_FAILURE;
        }
    }

    double t_init_end = get_time();
    printf("Initialization time: %.6f seconds\n", t_init_end - t_init_start);

    // Synchronize start
    spin_barrier_wait(&start_barrier);
    double t0 = get_time();

    // Wait for threads to finish measured loop
    spin_barrier_wait(&end_barrier);
    double t1 = get_time();

    double measured = t1 - t0;
    printf("Measured barrier loop time (threads=%d, iterations=%d): %.6f seconds\n", num_threads, iterations, measured);
    printf("Average time per barrier (all threads): %.9f seconds\n", measured / (double)iterations);

    // Join
    for (int i = 0; i < num_threads; i++) pthread_join(threads[i], NULL);

    // Cleanup
    spin_barrier_destroy(&start_barrier);
    spin_barrier_destroy(&measured_barrier);
    spin_barrier_destroy(&end_barrier);
    free(threads);
    return 0;
}