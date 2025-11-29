/*
 * assignment1/src/barriers/barrier_pthread.c
 * Exercise 1.5 - pthread barrier
 * Usage: ./bin/barrier_pthread <num_threads> <iterations>
 *
 * Implementation of a sense-reversal pthread barrier using pthreads.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

static double get_time(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}

static int num_threads = 4;
static int iterations = 1000000;

static pthread_barrier_t start_barrier;
static pthread_barrier_t barrier;
static pthread_barrier_t end_barrier;

void* thread_function(void* arg) {
    (void)arg; // thread id not used

    // wait until main and all threads are ready
    pthread_barrier_wait(&start_barrier);

    // measured loop: threads synchronize among themselves on 'barrier'
    for (int i = 0; i < iterations; i++) {
        pthread_barrier_wait(&barrier);
    }

    // signal completion
    pthread_barrier_wait(&end_barrier);
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
    int *thread_ids = malloc(sizeof(int) * num_threads);
    if (!threads || !thread_ids) {
        fprintf(stderr, "Allocation failed\n");
        return EXIT_FAILURE;
    }

    double t_init_start = get_time();

    // Initialize barriers: start and end include main (num_threads + 1), barrier is for threads only
    if (pthread_barrier_init(&start_barrier, NULL, (unsigned)(num_threads + 1)) != 0) {
        perror("pthread_barrier_init start_barrier");
        return EXIT_FAILURE;
    }
    if (pthread_barrier_init(&barrier, NULL, (unsigned)num_threads) != 0) {
        perror("pthread_barrier_init barrier");
        return EXIT_FAILURE;
    }
    if (pthread_barrier_init(&end_barrier, NULL, (unsigned)(num_threads + 1)) != 0) {
        perror("pthread_barrier_init end_barrier");
        return EXIT_FAILURE;
    }

    // Create threads; they will wait at start_barrier
    for (int i = 0; i < num_threads; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, thread_function, &thread_ids[i]) != 0) {
            perror("Failed to create thread");
            return EXIT_FAILURE;
        }
    }

    double t_init_end = get_time();
    printf("Initialization time: %.6f seconds\n", t_init_end - t_init_start);

    // Synchronize start: release threads and then start timer immediately after barrier returns
    pthread_barrier_wait(&start_barrier);
    double t0 = get_time();

    // Wait for threads to finish measured loop
    pthread_barrier_wait(&end_barrier);
    double t1 = get_time();

    double measured = t1 - t0;
    printf("Measured barrier loop time (threads=%d, iterations=%d): %.6f seconds\n", num_threads, iterations, measured);
    printf("Average time per barrier (all threads): %.9f seconds\n", measured / (double)iterations);

    // Join and cleanup
    for (int i = 0; i < num_threads; i++) pthread_join(threads[i], NULL);
    pthread_barrier_destroy(&start_barrier);
    pthread_barrier_destroy(&barrier);
    pthread_barrier_destroy(&end_barrier);
    free(threads);
    free(thread_ids);
    return 0;
}
