#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
// Bandwidth measurements

int main(int argc, char **argv){
    int message_size = atoi(argv[1]);
    char* data = malloc(message_size);

    int size, rank, i, j;
    
    double time;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, & rank);

    int count = 1e8 / message_size;

    time = MPI_Wtime();

        for(i = 0; i < count; ++i){
            if(rank == 0){
                MPI_Send(data, message_size, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
                MPI_Recv(data, message_size, MPI_CHAR, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            else if(rank == 1){
                MPI_Recv(data, message_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Send(data, message_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
            }
        }

        if (rank == 0) {
                time = MPI_Wtime() - time;
                FILE *f;
                f = fopen("time.csv", "a");
                assert(f);
                fprintf(f, "%f, %d \n", 0.5*(time)/count*1e6, message_size);
                fclose(f);
        }


    free(data);
    MPI_Finalize();
    return 0;
}
