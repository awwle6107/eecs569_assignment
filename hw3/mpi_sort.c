#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <mpi.h>
#include <math.h>

#define MSIZE 1024
#define ROOT 0

void shuffle(unsigned int *array, unsigned n);
int comparison(const void *a, const void *b);

int main(unsigned int argc, char *argv[])
{
	unsigned int N; // size of array

	// initialize MPI and get size/rank
	int mpi_rank, mpi_size;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

	// get size of array from command line
	if (argc < 2)
	{
		printf("Not enough arguments, defaulting to N=%d. Usage: [N] [(optional) data_filename]\n", MSIZE);
		N = MSIZE;
	}
	else
	{
		// get size of matrix
		N = atol(argv[1]);
	}

	unsigned int *arr;
	unsigned int *arr_sub;
	int num_elements_per_proc = N / mpi_size;
	arr_sub = malloc(sizeof(int) * num_elements_per_proc);
	double t;

	// create array in order:
	if (mpi_rank == ROOT)
	{
		arr = malloc(sizeof(int) * N);
		for (long i = 0; i < N; i++)
			arr[i] = i;

		// shuffle the array
		srand(time(NULL));
		shuffle(arr, N);
		printf("0: [%d], 1003: [%d]\n", arr[0], arr[1003]);

		double t = MPI_Wtime();
	}

	MPI_Scatter(
		arr, num_elements_per_proc, MPI_UNSIGNED,
		arr_sub, num_elements_per_proc, MPI_UNSIGNED,
		ROOT, MPI_COMM_WORLD);

	// sort the array
	qsort(arr_sub, num_elements_per_proc, sizeof(int), comparison);

	// Test to see how how many rounds there are - rounds should = log2(Processes)
	int test = mpi_size;
	int rounds = 1;
	while (test > 2)
	{
		test = test / 2;
		rounds++;
	}

	// Merge sort - combining the sub matrices
	int current_round = 0;
	int partner;
	int power_old;
	int power_new;
	unsigned int size_of_new_array;
	int size_of_old_array;
	for (int i = 0; i < rounds; i++)
	{
		current_round++;
		power_new = pow(2, current_round);		 // 2^(curent round)
		power_old = pow(2, (current_round - 1)); // 2^(last round)
		size_of_old_array = num_elements_per_proc * power_old;
		size_of_new_array = num_elements_per_proc * power_new;

		if (mpi_rank % power_new == 0)
		{
			partner = mpi_rank + power_old; // partner receiving from
			int *arr_rec;
			arr_rec = malloc(sizeof(int) * size_of_old_array);
			MPI_Recv(
				arr_rec, size_of_old_array, MPI_INT, partner,
				1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			unsigned int *arr_merge;
			arr_merge = malloc(sizeof(int) * size_of_new_array);

			// MERGE AND SORT ARRAYS, adapted from: https://github.com/jkndrkn/erlang-mpi/blob/master/mergesort.c 

			int sub_i;
			int rec_i;
			unsigned int merge_i;
			unsigned int i, k;
			sub_i = 0;
			rec_i = 0;
			merge_i = 0;

			while ((sub_i < size_of_old_array) && (rec_i < size_of_old_array))
			{
				if (arr_sub[sub_i] <= arr_rec[rec_i])
				{
					arr_merge[merge_i] = arr_sub[sub_i];
					merge_i++;
					sub_i++;
				}
				else
				{
					arr_merge[merge_i] = arr_rec[rec_i];
					merge_i++;
					rec_i++;
				}
			}
			// if get done sorting arr_sub first, fill the merge array with the remaining values in arr_rec
			if (sub_i >= size_of_old_array)
				for (i = merge_i; i < size_of_new_array; i++, rec_i++)
					arr_merge[i] = arr_rec[rec_i];
			// if get done sorting arr_rec first, fill the merge array with the remaining values in arr_sub
			else if (rec_i >= size_of_old_array)
				for (i = merge_i; i < size_of_new_array; i++, sub_i++)
					arr_merge[i] = arr_sub[sub_i];

			free(arr_sub);
			free(arr_rec);
			arr_sub = malloc(sizeof(int) * size_of_new_array);

			for (k = 0; k < size_of_new_array; k++)
				arr_sub[k] = arr_merge[k];
			free(arr_merge);
		}
		else if (mpi_rank % power_new == power_old)
		{
			partner = mpi_rank - power_old; // partner sending to
			int *arr_send = malloc(sizeof(int) * size_of_old_array);
			for (int j = 0; j < size_of_old_array; j++)
				arr_send[j] = arr_sub[j];
			MPI_Send(
				arr_send, size_of_old_array, MPI_INT, partner,
				1, MPI_COMM_WORLD);
		}
	}
	if (mpi_rank == ROOT)
	{
		t = MPI_Wtime() - t;

		printf("sorted %d numbers in %f s\n", N, t);
		printf("0: [%d], 1003: [%d]\n", arr_sub[0], arr_sub[1003]);

		//(optional) save metrics to file if argv[2] exists
		if (argc > 2)
		{ // assume (in production code, check) 2nd arg is datafile out
			FILE *csv_file;

			// first check if the file has already been created
			csv_file = fopen(argv[2], "r");
			if (!csv_file)
			{
				// not created, reopen file as write, write column headers
				csv_file = fopen(argv[2], "w");
				fprintf(csv_file, "N,P,s\n");
			}
			fclose(csv_file); // close outside brackets in case the fopen in read worked

			// file is created from here onward, open in append mode and add the data
			csv_file = fopen(argv[2], "a");
			fprintf(csv_file, "%d,%d,%f\n", N, mpi_size, t);
			fclose(csv_file);
		}
	}

	// shut down MPI
	if (mpi_rank == ROOT)
		free(arr);
	free(arr_sub);

	MPI_Finalize();

	return 0;
}

/* Arrange the N elements of ARRAY in random order.
   Only effective if N is much smaller than RAND_MAX;
   if this may not be the case, use a better random
   number generator. */

// From: https://benpfaff.org/writings/clc/shuffle.html
void shuffle(unsigned int *array, unsigned int n)
{
	if (n > 1)
	{
		long i;
		for (i = 0; i < n - 1; i++)
		{
			long j = i + rand() / (RAND_MAX / (n - i) + 1);
			int t = array[j];
			array[j] = array[i];
			array[i] = t;
		}
	}
}

// comparison function for qsort
// based on: https://www.tutorialspoint.com/c_standard_library/c_function_qsort.htm
int comparison(const void *a, const void *b)
{
	return (*(unsigned int *)a - *(unsigned int *)b);
}
