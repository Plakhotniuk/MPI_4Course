#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <mpi.h>

#define TIME 0
#define SAVE_VTK 1

#define ind(i, j) (((i + l->nx) % l->nx) + ((j + l->ny) % l->ny) * (l->nx))

typedef struct {
	int nx, ny;
	int *u0;
	int *u1;
	int steps;
	int save_steps;

    // Decomposition
    // N - data_plot size
    // P - number of all processes
    // K - current process num
    int rank; // current process number, K
    int num_tasks; // number of all processes, P
    int start; // start row index
    int finish; // end row index

    MPI_Datatype block_type; // standard block type for sending
    MPI_Datatype column_type; // single column type
    MPI_Datatype gather_type;

} life_t;


void life_init(const char *path, life_t *l);
void life_free(life_t *l);
void life_step(life_t *l);
void life_save_vtk(const char *path, life_t *l);
void decomposition(const int N, const int P, const int k, int *start, int *finish);
void gather_block_data(life_t *l); // gather standard block data_plot in last process
void gather_data(life_t *l);


int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

	if (argc != 2) {
		printf("Usage: %s input file.\n", argv[0]);
		return 0;
	}
	life_t l;
	life_init(argv[1], &l);
	
	int i;
	char buf[100];

    #if TIME 
	double time;
    if(l.rank == 0) time = MPI_Wtime();
	#endif

    for (i = 0; i < l.steps; i++) {
        if (i % l.save_steps == 0) {
            gather_data(&l); // gathering data_plot
#if SAVE_VTK
            if(l.rank == l.num_tasks - 1){
                sprintf(buf, "vtk/life_%06d.vtk", i);
                printf("Saving step %d to '%s'.\n", i, buf);
                life_save_vtk(buf, &l);
            }
#endif
        }
        life_step(&l);
    }


    #if TIME
	if (l.rank == 0) {
		time = MPI_Wtime() - time;
		FILE *f;
		f = fopen("data_plot/time.txt", "a");
		assert(f);
		fprintf(f, "%d %f\n", l.num_tasks, time);
		fclose(f);
	}
	#endif 

	life_free(&l);

    MPI_Finalize();
	return 0;
}

/** Gather data_plot in l->num_tasks - 1
 *
 * @param l
 */
void gather_block_data(life_t *l){
    if(l->rank == l->num_tasks - 1){
        int k;
        int start, finish;
        for (k = 0; k < l->num_tasks - 1; ++k){
            decomposition(l->nx, l->num_tasks, k, &start, &finish);
            MPI_Recv(l->u0 + ind(start, 0), 1, l->block_type, k, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    } else {
        MPI_Send(l->u0 + ind(l->start, 0), 1, l->block_type, l->num_tasks - 1, 0, MPI_COMM_WORLD);
    }
}

void gather_data(life_t *l)
{
    int start, finish;
    decomposition(l->nx, l->num_tasks, l->num_tasks - 1, &start, &finish);
//    printf("rank: %d, start: %d, gather start: %d\n", l->rank, l->start, start);
    MPI_Gather(l->u0 + ind(l->start, 0),
               1,
               l->gather_type,
               l->u0 + ind(start, 0),
               1,
               l->gather_type,
               l->num_tasks - 1,
               MPI_COMM_WORLD
    );
}

/** Column decomposition function
 *
 * @param N - number of columns
 * @param P - number of all processes
 * @param k - number of current process
 * @param start - pointer to start column
 * @param finish - pointer to finish column
 */
void decomposition(const int N, const int P, const int k, int *start, int *finish){
    // Assumption: N >> P, N % P << N
    int Nk = N / P;
    *start = k * Nk;
    *finish = *start + Nk;
    if(k == P - 1) *finish = N;
};


/**
 * Загрузить входную конфигурацию.
 * Формат файла, число шагов, как часто сохранять, размер поля, затем идут координаты заполненых клеток:
 * steps
 * save_steps
 * nx ny
 * i1 j2
 * i2 j2
 */
void life_init(const char *path, life_t *l)
{
	FILE *fd = fopen(path, "r");
	assert(fd);
	assert(fscanf(fd, "%d\n", &l->steps));
	assert(fscanf(fd, "%d\n", &l->save_steps));
	printf("Steps %d, save every %d step.\n", l->steps, l->save_steps);
	assert(fscanf(fd, "%d %d\n", &l->nx, &l->ny));
	printf("Field size: %dx%d\n", l->nx, l->ny);

	l->u0 = (int*)calloc(l->nx * l->ny, sizeof(int));
	l->u1 = (int*)calloc(l->nx * l->ny, sizeof(int));

	int i, j, r, cnt;
	cnt = 0;
	while ((r = fscanf(fd, "%d %d\n", &i, &j)) != EOF) {
		l->u0[ind(i, j)] = 1;
		cnt++;
	}
	printf("Loaded %d life cells.\n", cnt);
	fclose(fd);


    MPI_Comm_size(MPI_COMM_WORLD, &(l->num_tasks));
    MPI_Comm_rank(MPI_COMM_WORLD, &(l->rank));
    // Column decomposition
    decomposition(l->nx, l->num_tasks, l->rank, &(l->start), &(l->finish));

    // Create standard block for sending
    int start, finish;
    decomposition(l->nx, l->num_tasks, 0, &start, &finish);
    MPI_Type_vector(l->ny, finish - start, l->nx, MPI_INT, &(l->block_type));
    MPI_Type_commit(&(l->block_type));

    // Create single column for exchanges
    MPI_Type_vector(l->ny, 1, l->nx, MPI_INT, &(l->column_type));
    MPI_Type_commit(&(l->column_type));

    // For gather
    if(l->rank == l->num_tasks - 1){
        int dif = finish - start;
        int last_finish = dif * (l->num_tasks + 1);
        MPI_Type_create_resized(l->block_type, 0, (last_finish - l->start) * sizeof(int), &(l->gather_type));
    } else{
        MPI_Type_create_resized(l->block_type, 0, (l->finish - l->start) * sizeof(int), &(l->gather_type));
    }
    MPI_Type_commit(&(l->gather_type));
    
    printf("#%d: start = %d, finish = %d \n", l->rank, l->start, l->finish);

}

void life_free(life_t *l)
{
	free(l->u0);
	free(l->u1);
	l->nx = l->ny = 0;
    MPI_Type_free(&(l->block_type));
    MPI_Type_free(&(l->column_type));
    MPI_Type_free(&(l->gather_type));
}

void life_save_vtk(const char *path, life_t *l)
{

	FILE *f;
	int i1, i2;
	f = fopen(path, "w");
	assert(f);
	fprintf(f, "# vtk DataFile Version 3.0\n");
	fprintf(f, "Created by write_to_vtk2d\n");
	fprintf(f, "ASCII\n");
	fprintf(f, "DATASET STRUCTURED_POINTS\n");
	fprintf(f, "DIMENSIONS %d %d 1\n", l->nx+1, l->ny+1);
	fprintf(f, "SPACING %d %d 0.0\n", 1, 1);
	fprintf(f, "ORIGIN %d %d 0.0\n", 0, 0);
	fprintf(f, "CELL_DATA %d\n", l->nx * l->ny);
	
	fprintf(f, "SCALARS life int 1\n");
	fprintf(f, "LOOKUP_TABLE life_table\n");

	for (i2 = 0; i2 < l->ny; i2++) {
		for (i1 = 0; i1 < l->nx; i1++) {
			fprintf(f, "%d\n", l->u0[ind(i1, i2)]);
		}
	}
	fclose(f);

}


void life_step(life_t *l)
{

    int next_process = (l->rank + 1) % l->num_tasks; // next process index
    int prev_process = (l->rank + l->num_tasks - 1) % l->num_tasks; // prev process index

//    // column indexes own process
    int own_row_finish = ind(l->finish-1, 0);
    int own_row_start = ind(l->start, 0);
//
    int next_start, next_finish;
    decomposition(l->nx, l->num_tasks, next_process, &next_start, &next_finish);
//
    int prev_start, prev_finish;
    decomposition(l->nx, l->num_tasks, prev_process, &prev_start, &prev_finish);
//
    if (l->rank % 2 == 0) {
        MPI_Send(l->u0 + own_row_finish, 1, l->column_type, next_process, 0, MPI_COMM_WORLD);
        MPI_Recv(l->u0 + ind(next_start, 0), 1, l->column_type, next_process, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Send(l->u0 + own_row_start, 1, l->column_type, prev_process, 0, MPI_COMM_WORLD);
        MPI_Recv(l->u0 + ind(prev_finish - 1, 0), 1, l->column_type, prev_process, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    if (l->rank % 2 == 1) {
        MPI_Recv(l->u0 + ind(prev_finish - 1, 0), 1, l->column_type, prev_process, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Send(l->u0 + own_row_start, 1, l->column_type, prev_process, 0, MPI_COMM_WORLD);
        MPI_Recv(l->u0 + ind(next_start, 0), 1, l->column_type, next_process, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Send(l->u0 + own_row_finish, 1, l->column_type, next_process, 0, MPI_COMM_WORLD);
    }

	int i, j;

    for (j = 0; j < l->ny; j++) {
        for (i = l->start; i < l->finish; i++) {
            int n = 0;
            n += l->u0[ind(i+1, j)];
            n += l->u0[ind(i+1, j+1)];
            n += l->u0[ind(i,   j+1)];
            n += l->u0[ind(i-1, j)];
            n += l->u0[ind(i-1, j-1)];
            n += l->u0[ind(i,   j-1)];
            n += l->u0[ind(i-1, j+1)];
            n += l->u0[ind(i+1, j-1)];
            l->u1[ind(i,j)] = 0;
            if (n == 3 && l->u0[ind(i,j)] == 0) {
                l->u1[ind(i,j)] = 1;
            }
            if ((n == 3 || n == 2) && l->u0[ind(i,j)] == 1) {
                l->u1[ind(i,j)] = 1;
            }
            // Visual check
//            l->u1[ind(i,j)] = l->rank;
        }
    }

	int *tmp;
	tmp = l->u0;
	l->u0 = l->u1;
	l->u1 = tmp;
}
