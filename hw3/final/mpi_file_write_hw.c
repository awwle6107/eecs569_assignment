#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <mpi.h>

#define ROOT 0
#define R_CONST 100
#define MPI_FNAME "mpi_write.out"

// Creates an array of random numbers. Each number has a value from 0 - 99
float *create_rand_nums(long num_elements)
{
	long i;
	float *rand_nums = (float *)malloc(sizeof(float) * num_elements);

	for (i = 0; i < num_elements; i++)
	{
		rand_nums[i] = ((float)rand() / RAND_MAX) * 100;
	}

	return rand_nums;
}

int main(int argc, char *argv[])
{

	if (argc < 3)
	{
		// argc[1] is the filename from commandline
		printf("error: need at least two arguments. Usage: file_write [filename] [size] [optional: csv data filename]\n");
		exit(-1);
	}
	double t0;
	// initialize MPI and get size/rank
	int mpi_rank, mpi_size;

	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

	// get size of file as number of floats
	long int total_num = atol(argv[2]);
	long int each_num = (long int)atol(argv[2]) / mpi_size;
	double MB = sizeof(float) * total_num / pow(2,20);

	srand(time(NULL) * (mpi_rank + 1));

	// allocate memory for size floats
	float *partial_arrays = malloc(each_num * sizeof(float));
	// uniform randomly initialize values in array between -R_CONST and +R_CONST
	partial_arrays = create_rand_nums(each_num);

	MPI_Barrier(MPI_COMM_WORLD);
	if (mpi_rank == ROOT)
	{
		t0 = MPI_Wtime();
	}

	// //open file for write
	MPI_File fs;
	MPI_File_open(MPI_COMM_WORLD, argv[1], MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &fs);
	MPI_File_seek(fs, each_num * sizeof(float) * mpi_rank, MPI_SEEK_SET);
	MPI_File_write(fs, partial_arrays, 1, MPI_FLOAT, MPI_STATUS_IGNORE);
	MPI_File_close(&fs);

	// wait for all thread to finish writing
	MPI_Barrier(MPI_COMM_WORLD);
	if (mpi_rank == ROOT)
	{
		t0 = MPI_Wtime() - t0;
		printf("write time with %d threads = %f\n", mpi_size, t0);

		// write data to a csv file if argv[3] exists
		if (argc > 3)
		{					// assume (in production code, check) 3rd arg is datafile out
			FILE *csv_file; // could reuse arr_file as it is closed, but more readable this way

			// first check if the file has already been created
			csv_file = fopen(argv[3], "r");
			if (!csv_file)
			{
				// not created, reopen file as write, write column headers
				csv_file = fopen(argv[3], "w");
				fprintf(csv_file, "N,MB,s,MB/s,P\n");
			}
			fclose(csv_file); // close outside brackets in case the fopen in read worked

			// file is created from here onward, open in append mode and add the data
			csv_file = fopen(argv[3], "a");
			fprintf(csv_file, "%ld,%.2f,%.5f,%.2f,%d\n", total_num, MB, t0, MB / t0, mpi_size);
			fclose(csv_file);
		}
	}

	free(partial_arrays);
	MPI_Finalize();

	return 0;
}
