#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <mpi.h>
#define MSIZE 10000
#define ROOT 0
int main(int argc, char *argv[])
{
    int i, j, k, N;
    if (argc < 2)
    {
        printf("Not enough arguments, defaulting to N=%d. Usage: [N] [(optional) data_filename]\n", MSIZE);
        N = MSIZE;
    }
    else
    {
        // get size of matrix
        N = atoi(argv[1]);
    }
    double t0;
    int mpi_rank, mpi_size;
    MPI_Init(&argc,&argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

    int partial_N = N / mpi_size;
    int total = 0;
    int partial_total = 0;
    MPI_Bcast(&partial_total,1,MPI_INT,ROOT,MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    if (mpi_rank == ROOT)
    {
        t0 = MPI_Wtime();
    }
    for(int i = mpi_rank * partial_N; i < (mpi_rank + 1) * partial_N; i++){
        int prime = 1;
        if(i < 2){
            continue;
        }
        for(j = 2; j < i; j++){
            if ((i % j) == 0)
            {
                prime = 0;
                break;
            }
        }
        partial_total = partial_total + prime;
    }
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Reduce(&partial_total,&total,1,MPI_INT,MPI_SUM,ROOT,MPI_COMM_WORLD);
    if(mpi_rank == ROOT){
        printf("there are %d primes under %d\n", total, N);
        t0 = MPI_Wtime() - t0;
		printf("time with %d threads = %f\n", mpi_size, t0);

        if (argc > 2)
		{ // assume (in production code, check) 2nd arg is datafile out
			FILE *csv_file;

			// first check if the file has already been created
			csv_file = fopen(argv[2], "r");
			if (!csv_file)
			{
				// not created, reopen file as write, write column headers
				csv_file = fopen(argv[2], "w");
				fprintf(csv_file, "N,s,P\n");
			}
			fclose(csv_file); // close outside brackets in case the fopen in read worked

			// file is created from here onward, open in append mode and add the data
			csv_file = fopen(argv[2], "a");
			fprintf(csv_file, "%d,%.5f,%d\n", N,t0, mpi_size);
			fclose(csv_file);
		}
    }
    MPI_Finalize();
	return 0;
}