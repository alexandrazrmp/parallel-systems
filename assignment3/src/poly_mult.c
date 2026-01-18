#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
    int rank, nprocs, n, i, j, k;
    int64_t *A = NULL, *B = NULL, *C = NULL, *local_C = NULL;
    // Αρχικοποίηση για την αποφυγή των warnings "maybe-uninitialized"
    double t1 = 0.0, t2 = 0.0, t_send = 0.0, t_comp = 0.0, t_recv = 0.0;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    
    if (argc != 2) {
        if (rank == 0) printf("Usage: %s <n>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }
    
    n = atoi(argv[1]);
    int res_sz = 2 * n + 1;
    
    // Όλες οι διεργασίες δεσμεύουν μνήμη για τα αρχικά πολυώνυμα A και B
    A = malloc((n + 1) * sizeof(int64_t));
    B = malloc((n + 1) * sizeof(int64_t));
    
    if (rank == 0) {
        C = malloc(res_sz * sizeof(int64_t));
        if (!A || !B || !C) {
            fprintf(stderr, "Allocation failed on root\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        
        srand(42);
        for (i = 0; i <= n; i++) {
            A[i] = (rand() % 20) - 10;
            B[i] = (rand() % 20) - 10;
            if (A[i] == 0) A[i] = 1;
            if (B[i] == 0) B[i] = 1;
        }
    }
    
    // Συγχρονισμός πριν την έναρξη των μετρήσεων
    MPI_Barrier(MPI_COMM_WORLD);
    
    // 1. Χρόνος Αποστολής (Broadcast)
    if (rank == 0) t1 = MPI_Wtime();
    
    MPI_Bcast(A, n + 1, MPI_INT64_T, 0, MPI_COMM_WORLD);
    MPI_Bcast(B, n + 1, MPI_INT64_T, 0, MPI_COMM_WORLD);
    
    if (rank == 0) t_send = MPI_Wtime() - t1;
    
    // Υπολογισμός ορίων για κάθε διεργασία (Load Balancing)
    int chunk = res_sz / nprocs;
    int rem = res_sz % nprocs;
    int my_start, my_cnt;
    
    if (rank < rem) {
        my_cnt = chunk + 1;
        my_start = rank * my_cnt;
    } else {
        my_cnt = chunk;
        my_start = rank * chunk + rem;
    }
    
    local_C = malloc(my_cnt * sizeof(int64_t));
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    // 2. Χρόνος Υπολογισμού
    t1 = MPI_Wtime();
    
    for (k = 0; k < my_cnt; k++) {
        i = my_start + k;
        int64_t sum = 0;
        int jmin = (i > n) ? i - n : 0;
        int jmax = (i < n) ? i : n;
        for (j = jmin; j <= jmax; j++) {
            sum += A[j] * B[i - j];
        }
        local_C[k] = sum;
    }
    
    t2 = MPI_Wtime();
    double my_comp_time = t2 - t1;
    
    // Παίρνουμε τον μέγιστο χρόνο υπολογισμού από όλες τις διεργασίες
    MPI_Reduce(&my_comp_time, &t_comp, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    
    // 3. Χρόνος Λήψης (Gather)
    int *cnts = NULL, *disps = NULL;
    if (rank == 0) {
        cnts = malloc(nprocs * sizeof(int));
        disps = malloc(nprocs * sizeof(int));
        int d = 0;
        for (i = 0; i < nprocs; i++) {
            cnts[i] = chunk + (i < rem ? 1 : 0);
            disps[i] = d;
            d += cnts[i];
        }
        t1 = MPI_Wtime();
    }
    
    MPI_Gatherv(local_C, my_cnt, MPI_INT64_T, C, cnts, disps, MPI_INT64_T, 0, MPI_COMM_WORLD);
    
    if (rank == 0) {
        t_recv = MPI_Wtime() - t1;
        
        // Εκτύπωση αποτελεσμάτων
        printf("N=%d, Processes=%d\n", n, nprocs);
        printf("Communication Send Time: %.6f sec\n", t_send);
        printf("Computation Time (Max):  %.6f sec\n", t_comp);
        printf("Communication Recv Time: %.6f sec\n", t_recv);
        printf("Total Execution Time:    %.6f sec\n", t_send + t_comp + t_recv);
        
        // Επιβεβαίωση (Verification)
        int ok = 1;
        for (i = 0; i < res_sz && ok; i++) {
            int64_t s = 0;
            int jmin = (i > n) ? i - n : 0;
            int jmax = (i < n) ? i : n;
            for (j = jmin; j <= jmax; j++)
                s += A[j] * B[i - j];
            if (C[i] != s) ok = 0;
        }
        printf("Verification Status: %s\n", ok ? "PASS" : "FAIL");
        
        free(C);
        free(cnts);
        free(disps);
    }
    
    free(A);
    free(B);
    free(local_C);
    
    MPI_Finalize();
    return 0;
}