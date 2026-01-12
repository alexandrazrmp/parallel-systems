#include <stdio.h>
#include <unistd.h>
#include <mpi.h>

int main(int argc, char** argv) {
    int rank, size;
    char hostname[256];

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    gethostname(hostname, 256);

    printf("[sparse_mv] Rank %d of %d running on %s\n",
           rank, size, hostname);

    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0) {
        printf("[sparse_mv] All processes finished successfully.\n");
    }

    MPI_Finalize();
    return 0;
}
