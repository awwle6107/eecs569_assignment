#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <mpi.h>
// comment to silence statistics of performance
#define VERBOSE

// default size of matrices are MSIZE x MSIZE
#define MSIZE 4096

// range of random numbers
#define R_CONST 100
#define ROOT 0
// macro to index into matrix at location (i,j)
#define index(i, j, rowsize) i *rowsize + j

// seed for random
#define SEED 10

void randomizeMatrix(float range, int N, float *matrix)
{
	// uniform randomly initialize values in array between -range and +range
	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
		{
			matrix[index(i, j, N)] = (((float)rand() / (float)RAND_MAX) * 2 * range) - range;
		}
}

void transpose(float *matrix, float *matrix_t, int N)
{
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			matrix_t[index(i, j, N)] = matrix[index(j, i, N)];
		}
	}
}

void print_matrix(float *matrix, int N)
{
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			printf("%0.2f ", matrix[index(i, j, N)]);
		}
		printf("\n");
	}
	printf("\n");
}

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

	srand(SEED); // seed the RNG

	// initialize memory
	float *matA;
	float *matB;
	float *matC;
	float *matA_t;

	// initialize the mat A B C
	matA = malloc(N * N * sizeof(float));
	matB = malloc(N * N * sizeof(float));
	matC = calloc(N * N, sizeof(float));

	// randomize A and B
	randomizeMatrix(R_CONST, N, matA);
	randomizeMatrix(R_CONST, N, matB);

	// do naive matrix multiply
	//  for (i = 0; i < N; i++){
	//  	for (j = 0; j < N; j++){
	//  		float val = 0;
	//  		for (k = 0; k < N; k++)
	//  			val += matA[index(i,k,N)]*matB[index(k,j,N)];
	//  		matC[index(i,j,N)] = val;
	//  	}
	//  }
	//  printf("Non serial: \n");
	//  print_matrix(matC, N);
	//  free(matC);
	//  matC = calloc(N*N,sizeof(float));

	// get start time
	double t0;
	int mpi_rank, mpi_size;
	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
	int scatterCount = N * N / mpi_size;
	// sub matricies to store for scattered from main matrices
	float *matAA;
	float *matCC;
	matAA = malloc(scatterCount * sizeof(float));
	matCC = malloc(scatterCount * sizeof(float));

	// scatter rows of matA to processes based on the mpi_size
	if (mpi_rank == ROOT)
	{
		t0 = MPI_Wtime();
	}
	MPI_Scatter(matA, scatterCount, MPI_FLOAT,
				matAA, scatterCount, MPI_FLOAT,
				ROOT, MPI_COMM_WORLD);

	// broadcast matB to all the processes
	MPI_Bcast(matB, N * N, MPI_FLOAT, ROOT, MPI_COMM_WORLD);

	MPI_Barrier(MPI_COMM_WORLD);

	// mpi matrix multiplication

	float sum = 0.0;
	for (int k = 0; k < N / mpi_size; k++)
	{
		for (int i = 0; i < N; i++)
		{
			for (int j = 0; j < N; j++)
			{
				sum += matAA[index(k, j, N)] * matB[index(j, i, N)];
			}
			// may need to change the N
			matCC[index(k, i, N)] = sum;
			sum = 0.0;
		}
	}

	// gather sub-matrices from the buffer and store in matC
	MPI_Gather(matCC, scatterCount, MPI_FLOAT, matC, scatterCount, MPI_FLOAT, ROOT, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);
	if (mpi_rank == ROOT)
	{
		t0 = MPI_Wtime() - t0;
		// calculate metrics
		long FLOP = 2 * pow(N, 3);
		double Flops = FLOP / t0;

		printf("Performed a %d x %d matrix multiply in %.5f seconds\n", N, N, t0);
		printf("Number of floating point operations = 2 * %d^3 = %ld\n", N, FLOP);
		printf("Flops = %.3e\n", Flops);

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
				fprintf(csv_file, "N, FLOP, Flops, s, P\n");
			}
			fclose(csv_file); // close outside brackets in case the fopen in read worked

			// file is created from here onward, open in append mode and add the data
			csv_file = fopen(argv[2], "a");
			fprintf(csv_file, "%d,%ld,%.3e,%.5f,%d\n", N, FLOP, Flops,t0, mpi_size);
			fclose(csv_file);
		}
	}

	// free memory
	free(matA);
	free(matB);
	free(matC);
	free(matAA);
	free(matCC);
	MPI_Finalize();

	return 0;
}
