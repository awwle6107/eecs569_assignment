#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mpi.h>

#define ROOT 0

float *allocateFloatVector(long num)
{
	// initialize matrix
	float *vect = (float *)calloc(num, sizeof(float));
	return vect;
}

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		// argc[1] is the filename from commandline
		printf("error: need at least two arguments. Usage: file_read [filename] [size] [optional: csv data filename]\n");
		exit(-1);
	}
	int mpi_rank, mpi_size;
	double t0;
	// get size of file as number of floats
	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

	// get size of file as number of floats
	long int total_num = atol(argv[2]);
	long int each_num = total_num / mpi_size;
	double MB = sizeof(float) * total_num / pow(2,20);

	MPI_Barrier(MPI_COMM_WORLD);

	// allocate memory for size floats
	float *partial_array;
	partial_array = allocateFloatVector(each_num);

	MPI_Barrier(MPI_COMM_WORLD);
	// sequential read on root
	if (mpi_rank == ROOT)
	{
		t0 = MPI_Wtime();
	}
	// open file for read
	MPI_File fs;
	MPI_File_open(MPI_COMM_WORLD, argv[1], MPI_MODE_RDONLY, MPI_INFO_NULL, &fs);
	MPI_File_seek(fs, each_num * sizeof(float) * mpi_rank, MPI_SEEK_SET);
	MPI_File_read(fs, partial_array, 1, MPI_FLOAT, MPI_STATUS_IGNORE);
	MPI_File_close(&fs);
    MPI_Barrier(MPI_COMM_WORLD);
	// // write out file to see if read the right amount of bytes
	// MPI_File_open(MPI_COMM_WORLD,"test.out", MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &fs);
	// MPI_File_seek(fs, each_num * sizeof(float) * mpi_rank, MPI_SEEK_SET);
	// MPI_File_write(fs, partial_array, 1, MPI_FLOAT, MPI_STATUS_IGNORE);
	// MPI_File_close(&fs);
	// MPI_Barrier(MPI_COMM_WORLD);
	// // test end
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
	// // time read, calculate performance
	// double t = omp_get_wtime();
	// fread(array, sizeof(float), size, arr_file);
	// t = omp_get_wtime() - t;
	// double MB = sizeof(float) * size / pow(10, 6); // alternatively can use 2^20

	// printf("Read %f MB in %f seconds at a rate of %f MB/s\n", MB, t, MB / t);

	// // free array, close file
	// free(array);
	// fclose(arr_file);

	// // write data to a csv file if argv[3] exists
	// if (argc > 3)
	// {					// assume (in production code, check) 3rd arg is datafile out
	// 	FILE *csv_file; // could reuse arr_file as it is closed, but more readable this way

	// 	// first check if the file has already been created
	// 	csv_file = fopen(argv[3], "r");
	// 	if (!csv_file)
	// 	{
	// 		// not created, reopen file as write, write column headers
	// 		csv_file = fopen(argv[3], "w");
	// 		fprintf(csv_file, "N,MB,s,MB/s\n");
	// 	}
	// 	fclose(csv_file); // close outside brackets in case the fopen in read worked

	// 	// file is created from here onward, open in append mode and add the data
	// 	csv_file = fopen(argv[3], "a");
	// 	fprintf(csv_file, "%d,%f,%f,%f\n", size, MB, t, MB / t);
	// 	fclose(csv_file);
	// }
	free(partial_array);
	MPI_Finalize();
	return 0;
}
