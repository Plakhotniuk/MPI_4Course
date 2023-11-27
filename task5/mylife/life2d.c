#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <mpi.h>

#define TIME 0
#define SAVE_VTK 1

#define SEND_UP 1
#define SEND_DOWN 0
#define RECEIVE_UP 0
#define RECEIVE_DOWN 1

#define ind(i, j) (((i + l->nx) % l->nx) + ((j + l->ny) % l->ny) * (l->nx))

typedef struct {
	int nx, ny;
	int *u0;
	int *u1;
	int steps;
	int save_steps;

    // Decomposition
    // N - data size
    // P - number of all processes
    // K - current process num
    int rank; // current process number, K
    int num_tasks; // number of all processes, P
    int start[2]; // start 2d index
    int finish[2]; // finish 2d index
    int coords[2]; // coordinates in cartesian topology
    int dims[2]; // cartesian topology size
    /* Cartesian communicator. */
    MPI_Comm comm_cart;
    MPI_Comm comm_dims[2]; // row, col communicators

    // New types
    MPI_Datatype block_t;
    MPI_Datatype rows_t;

    // Types for exchange
    MPI_Datatype single_column;
    MPI_Datatype single_row;

} life_t;


void life_init(const char *path, life_t *l);
void life_free(life_t *l);
void life_step(life_t *l);
void life_save_vtk(const char *path, life_t *l);
void decomposition(const int N, const int P, const int k, int *start, int *finish);
void life_gather(life_t *l);
void life_gather_1d(life_t *l, MPI_Comm comm, MPI_Datatype block_t, int ind_send, int ind_recv);
void exchange_row(life_t *l);
void exchange_column(life_t *l);

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
            life_gather(&l);
#if SAVE_VTK
            if(l.rank == l.num_tasks - 1){
                sprintf(buf, "vtk/life_%06d.vtk", i);
                printf("Saving step %d to '%s'.\n", i, buf);
                life_save_vtk(buf, &l);
            }
#endif
        }
        life_step(&l);
        exchange_row(&l);
    }


    #if TIME
	if (l.rank == 0) {
		time = MPI_Wtime() - time;
		FILE *f;
		f = fopen("data/time.txt", "a");
		assert(f);
		fprintf(f, "%d %f\n", l.num_tasks, time);
		fclose(f);
	}
	#endif 

	life_free(&l);

    MPI_Finalize();
	return 0;
}


void life_gather(life_t *l){
    //printf("%d\n", l->start[1]);
    life_gather_1d(l, l->comm_dims[0], l->block_t, ind(l->start[0], l->start[1]), ind(0, l->start[1]));
    life_gather_1d(l, l->comm_dims[1], l->rows_t, ind(0, l->start[1]), ind(0, 0));
}

void life_gather_1d(life_t *l, MPI_Comm comm, MPI_Datatype block_t, int ind_send, int ind_recv)
{
    int size;
    MPI_Comm_size(comm, &size);
    MPI_Gather(l->u0 + ind_send,
               1,
               block_t,
               l->u0 + ind_recv,
               1,
               block_t,
               size - 1,
               comm
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

    /* Decomposition. */
    MPI_Comm_rank(MPI_COMM_WORLD, &(l->rank));
    MPI_Comm_size(MPI_COMM_WORLD, &(l->num_tasks));
    l->dims[0] = l->dims[1] = 0;
    MPI_Dims_create(l->num_tasks, 2, l->dims);

    int periods[2] = {1, 1};
    MPI_Cart_create(MPI_COMM_WORLD, 2, l->dims, periods, 0, &(l->comm_cart));
    MPI_Cart_coords(l->comm_cart, l->rank, 2, l->coords);
    printf("#%d: coords = {%d, %d}\n", l->rank, l->coords[0], l->coords[1]);
    decomposition(l->nx, l->dims[0], l->coords[0], l->start, l->finish);
    decomposition(l->ny, l->dims[1], l->coords[1], l->start+1, l->finish+1);


    /* Row/col communicators. */
    MPI_Comm_split(l->comm_cart, l->coords[0], l->coords[1], l->comm_dims + 0); // rows
    MPI_Comm_split(l->comm_cart, l->coords[1], l->coords[0], l->comm_dims + 1); // cols

    int start[2], finish[2];
    decomposition(l->nx, l->dims[0], 0, start, finish);
    decomposition(l->ny, l->dims[1], l->coords[1], start+1, finish+1);
    
    // column block for gather
    MPI_Datatype temp;
    MPI_Type_vector(finish[1] - start[1], finish[0] - start[0], l->nx, MPI_INT, &temp);
    MPI_Type_create_resized(temp, 0, (finish[0] - start[0]) * sizeof(int), &(l->block_t));
    MPI_Type_commit(&(l->block_t));
    
    // row block for gather
    decomposition(l->ny, l->dims[1], 0, start+1, finish+1);
    MPI_Type_contiguous(l->nx * (finish[1] - start[1]), MPI_INT, &(l->rows_t));
    MPI_Type_commit(&(l->rows_t));
    
    // row for exchange
    MPI_Type_contiguous(l->finish[0] - l->start[0], MPI_INT, &(l->single_row));
    MPI_Type_commit(&(l->single_row));

    // column for exchange
    MPI_Type_vector(l->finish[1] - l->start[1], 1, l->nx, MPI_INT, &(l->single_column));
    MPI_Type_commit(&(l->single_column));

}

void life_free(life_t *l)
{
	free(l->u0);
	free(l->u1);
	l->nx = l->ny = 0;
    MPI_Type_free(&(l->block_t));
    MPI_Type_free(&(l->rows_t));
    MPI_Type_free(&(l->single_row));
    MPI_Type_free(&(l->single_column));

    MPI_Comm_free(&(l->comm_dims[0]));
    MPI_Comm_free(&(l->comm_dims[1]));
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

void exchange_row(life_t *l){

    int rank;
    MPI_Comm_rank(l->comm_dims[0], &rank); // rank in column

    int next_process = (rank + 1) % l->dims[1]; // next process index
    int prev_process = (rank + l->dims[1] - 1) % l->dims[1]; // prev process index

//    // row indexes own process
    int own_row_finish = ind(l->start[0], l->finish[1]-1);
    int own_row_start = ind(l->start[0], l->start[1]);

    int next_start, next_finish;
    decomposition(l->ny, l->dims[1], next_process, &next_start, &next_finish);

    int prev_start, prev_finish;
    decomposition(l->ny, l->dims[1], prev_process, &prev_start, &prev_finish);

//    printf("rank: %d, own_row_start: %d, own_row_finish: %d \n",l->rank, own_row_start, own_row_finish);

    if (l->rank % 2 == 0) {
        MPI_Send(l->u0 + own_row_finish, 1, l->single_row, next_process, 0, l->comm_dims[1]);
        printf("Exchange. rank %d \n", l->rank);
        MPI_Recv(l->u0 + ind(l->start[0], next_start), 1, l->single_row, next_process, 0, l->comm_dims[1], MPI_STATUS_IGNORE);
        MPI_Send(l->u0 + own_row_start, 1, l->single_row, prev_process, 0, l->comm_dims[1]);
        MPI_Recv(l->u0 + ind(l->start[0], prev_finish - 1), 1, l->single_row, prev_process, 0, l->comm_dims[1], MPI_STATUS_IGNORE);

    }
    if (l->rank % 2 == 1) {
        MPI_Recv(l->u0 + ind(l->start[0], prev_finish - 1), 1, l->single_row, prev_process, 0, l->comm_dims[1], MPI_STATUS_IGNORE);
        printf("Exchange. rank %d \n", l->rank);
        MPI_Send(l->u0 + own_row_start, 1, l->single_row, prev_process, 0, l->comm_dims[1]);
        MPI_Recv(l->u0 + ind(l->start[0], next_start), 1, l->single_row, next_process, 0, l->comm_dims[1], MPI_STATUS_IGNORE);
        MPI_Send(l->u0 + own_row_finish, 1, l->single_row, next_process, 0, l->comm_dims[1]);
    }

}


void life_step(life_t *l)
{

	int i, j;

    for (j = l->start[1]; j < l->finish[1]; j++) {
        for (i = l->start[0]; i < l->finish[0]; i++) {
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
            //l->u1[ind(i,j)] = l->rank;
        }
    }

	int *tmp;
	tmp = l->u0;
	l->u0 = l->u1;
	l->u1 = tmp;
}
