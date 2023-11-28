#include <mpi.h>
#include <stdio.h>
// Latency measurements

int main(int argc, char **argv){
    char data = '0';
    int data_size = sizeof(data);
    int size, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, & rank);
    double t = MPI_Wtime();
    int count = 1e6;
    int message_size = 1e6;

    for(int i = 0; i < count; ++i){
        if(rank == 0){
            MPI_Send(&data, 1, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
            MPI_Recv(&data, 1, MPI_CHAR, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        else if(rank == 1){
            MPI_Recv(&data, 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(&data, 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
        }
    }

    if (rank == 0) printf("Latency = %f mk sec", 0.5*(MPI_Wtime() - t)/count*1e6);
    
    MPI_Finalize();
    return 0;
}
