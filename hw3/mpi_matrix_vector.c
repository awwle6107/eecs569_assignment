#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <mpi.h>

//comment to silence statistics of performance 
// #define VERBOSE

//default size of matrices are MSIZE x MSIZE
#define MSIZE 4096
#define ROOT 0
//range of random numbers
#define R_CONST 100 

//macro to index into matrix at location (i,j)
#define index(i,j,rowsize) i*rowsize+j

void randomizeMatrix(float range, int N, float * matrix){
	//uniform randomly initialize values in array between -range and +range
	for(int i = 0; i < N; i++)
		for(int j = 0; j < N; j++){
			matrix[index(i,j,N)] = (((float)rand()/(float)RAND_MAX) * 2 * range) - range;
		}
}

void randomizeVector(float range, int N, float * vec){
	//uniform randomly initialize values in array between -range and +range
	for(int i = 0; i < N; i++)
		vec[i] = (((float)rand()/(float)RAND_MAX) * 2 * range) - range;
}

void print_vector(float * vector, int N)
{
	for(int i=0; i < N; i++){
		printf("%.2f ", vector[i]);
	}
	printf("\n");
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
	int i,j,N;
	
	if(argc < 2){
		printf("Not enough arguments, defaulting to N=%d. Usage: [N] [(optional) data_filename]\n",MSIZE);
		N = MSIZE;
	}else{
		//get size of matrix 
		N = atoi(argv[1]);
	}
	
	srand(time(NULL)); //seed the RNG
	
	//initialize memory 
	float * matA;
	float * vecx;
	float * vecb;
	
	matA = malloc(N*N*sizeof(float));
	vecx = malloc(N*  sizeof(float));
	vecb = calloc(N  ,sizeof(float));
	
	//randomize A and x
	randomizeMatrix(R_CONST,N,matA);
	randomizeVector(R_CONST,N,vecx);
		
	// //do naive matrix-vector product
	// for (i = 0; i < N; i++){
	// 	for (j = 0; j < N; j++){
	// 		vecb[i] += matA[index(i,j,N)]*vecx[j];
	// 	}
	// }

	// vecb = calloc(N  ,sizeof(float));
	// mpi session

	double t0;
	int mpi_rank, mpi_size;
	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
	int matScatterCount = N * N / mpi_size;
	int vecScatterCount = N / mpi_size;
	float * matAA;
	float *vecbb;
	matAA = malloc(matScatterCount * sizeof(float));
	vecbb = malloc(vecScatterCount * sizeof(float));
	if (mpi_rank == ROOT)
	{
		t0 = MPI_Wtime();
	}
	MPI_Scatter(matA, matScatterCount, MPI_FLOAT,
				matAA, matScatterCount, MPI_FLOAT,
				ROOT, MPI_COMM_WORLD);

	MPI_Bcast(vecx, N, MPI_FLOAT, ROOT, MPI_COMM_WORLD);

	MPI_Barrier(MPI_COMM_WORLD);

	// do submatricies multiplication

	for (int i = 0; i < N / mpi_size; i++){
		for (int j = 0; j < N; j++){
			vecbb[i] += matAA[index(i,j,N)]*vecx[j];
		}
	}

	MPI_Gather(vecbb, vecScatterCount, MPI_FLOAT, vecb, vecScatterCount, MPI_FLOAT, ROOT, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);
	if (mpi_rank == ROOT)
	{
		t0 = MPI_Wtime() - t0;
		// calculate metrics
		long FLOP = 2*pow(N,2) - N;
		double Flops = FLOP / t0;

		printf("Performed a %d x %d matrix multiply in %.5f seconds\n", N, N, t0);
		printf("Number of floating point operations = 2 * %d^2 = %ld\n", N, FLOP);
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

	// sub matricies to store

	// //calculate metrics 
	// long FLOP = 2*pow(N,2) - N; 
	// double Flops = FLOP/time;
	
	// // get the num of threads from enviroment
	// char* OMP_NUM_THREADS = getenv("OMP_NUM_THREADS");
	// printf("OMP_NUM_THREADS: %s\n\n", OMP_NUM_THREADS );

	// #ifdef VERBOSE //by default not included 
	// printf("Performed a %d x %d matrix-vector product in %f seconds\n", N, N, time);
	// printf("Number of floating point operations = 2 * %d^3 = %ld\n", N, FLOP);
	// printf("MFlops = %.2f\n", Flops/pow(10,6));
	// #endif
	
	// //(optional) save metrics to file if argv[2] exists
	// if(argc > 2){ //assume (in production code, check) 2nd arg is datafile out 
	// 	FILE * csv_file;
		
	// 	//first check if the file has already been created
	// 	csv_file = fopen(argv[2],"r");
	// 	if(!csv_file){
	// 		//not created, reopen file as write, write column headers
	// 		csv_file = fopen(argv[2],"w");
	// 		fprintf(csv_file, "N,FLOP,Flops,s,T\n");
	// 	}
	// 	fclose(csv_file); //close outside brackets in case the fopen in read worked
		
	// 	// file is created from here onward, open in append mode and add the data 
	// 	csv_file = fopen(argv[2],"a");
	// 	fprintf(csv_file, "%d,%ld,%e,%lf,%s\n",N,FLOP,Flops,time,OMP_NUM_THREADS);
	// 	fclose(csv_file);
	// }
	
	//free memory
	free(matA);
	free(vecx);
	free(vecb);
	MPI_Finalize();
	return 0;
	
}
