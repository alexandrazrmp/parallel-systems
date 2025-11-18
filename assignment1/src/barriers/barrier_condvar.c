#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

// Usage: ./bin/barrier_condvar <num_threads> <iterations>

static double get_time(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int parties;        // total participants
    int count;          // threads still to arrive in this generation
    int generation;     // increments when barrier is released
} cond_barrier_t;

static void cond_barrier_init(cond_barrier_t *bar, int parties) {
    pthread_mutex_init(&bar->mutex, NULL);
    pthread_cond_init(&bar->cond, NULL);
    bar->parties = parties;
    bar->count = parties;
    bar->generation = 0;
}

static void cond_barrier_destroy(cond_barrier_t *bar) {
    pthread_mutex_destroy(&bar->mutex);
    pthread_cond_destroy(&bar->cond);
}

static void cond_barrier_wait(cond_barrier_t *bar) {
    pthread_mutex_lock(&bar->mutex);
    int my_generation = bar->generation;
    bar->count--;
    if (bar->count == 0) {
        bar->generation++;
        bar->count = bar->parties;
        pthread_cond_broadcast(&bar->cond);
    } else {
        while (my_generation == bar->generation) {
            pthread_cond_wait(&bar->cond, &bar->mutex);
        }
    }
    pthread_mutex_unlock(&bar->mutex);
}

static int num_threads = 4;
static int iterations = 1000000;

static cond_barrier_t start_barrier;
static cond_barrier_t measured_barrier;
static cond_barrier_t end_barrier;

void* thread_function(void* arg) {
    (void)arg; // thread id not used

    cond_barrier_wait(&start_barrier);

    for (int i = 0; i < iterations; i++) {
        cond_barrier_wait(&measured_barrier);
    }

    cond_barrier_wait(&end_barrier);
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

    cond_barrier_init(&start_barrier, num_threads + 1); // include main
    cond_barrier_init(&measured_barrier, num_threads);  // worker threads only
    cond_barrier_init(&end_barrier, num_threads + 1);   // include main

    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&threads[i], NULL, thread_function, NULL) != 0) {
            perror("Failed to create thread");
            return EXIT_FAILURE;
        }
    }

    double t_init_end = get_time();
    printf("Initialization time: %.6f seconds\n", t_init_end - t_init_start);

    cond_barrier_wait(&start_barrier);
    double t0 = get_time();

    cond_barrier_wait(&end_barrier);
    double t1 = get_time();

    double measured = t1 - t0;
    printf("Measured barrier loop time (threads=%d, iterations=%d): %.6f seconds\n", num_threads, iterations, measured);
    printf("Average time per barrier (all threads): %.9f seconds\n", measured / (double)iterations);

    for (int i = 0; i < num_threads; i++) pthread_join(threads[i], NULL);

    cond_barrier_destroy(&start_barrier);
    cond_barrier_destroy(&measured_barrier);
    cond_barrier_destroy(&end_barrier);
    free(threads);
    return 0;
}
