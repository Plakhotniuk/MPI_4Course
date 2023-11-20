#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
// Latency measurements

int main(int argc, char **argv){
    char* data[1000000];
    data[0] = (char*)malloc(1000000 * sizeof(char));

    int data_size = sizeof(data);
    int size, rank, i, j;
    
    double tStart, tEnd, time;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, & rank);

    int count = 1e3;
    time = MPI_Wtime();

    for(j = 1; j < 100000; j++){
        for(i = 0; i < count; ++i){
            if(rank == 0){
                MPI_Send(&data[0], j, MPI_CHAR, size - 1, 0, MPI_COMM_WORLD);    
            }
            else {
                MPI_Recv(&data[0], j, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }

        if (rank == 0) {
                time = MPI_Wtime() - time;
                FILE *f;
                f = fopen("time.csv", "a");
                assert(f);
                fprintf(f, "%f, %d \n", 0.5*(time)/count*1e6, j);
                fclose(f);
                // printf("%f, %d \n", 0.5*(tEnd-tStart)/count*1e6, j);
        }

    }
    
    MPI_Finalize();
    return 0;
}
