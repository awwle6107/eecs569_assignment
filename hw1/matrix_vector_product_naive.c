#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <omp.h>

//comment to silence statistics of performance 
#define VERBOSE

//default size of N are VSIZE
#define VSIZE 4096

//range of random numbers
#define R_CONST 100

//macro to index into vector at location (i,j)
#define index(i,j,rowsize) i*rowsize+j

void randomizeVector(float range, int N, float * vector){
	//uniform randomly initialize values in array between -range and +range
	for(int i = 0; i < N; i++)
		vector[i] = (((float)rand()/(float)RAND_MAX) * 2 * range) - range;
}

void randomizeMatrix(float range, int N, float * matrix){
	//uniform randomly initialize values in array between -range and +range
	for(int i = 0; i < N; i++)
		for(int j = 0; j < N; j++){
			matrix[index(i,j,N)] = (((float)rand()/(float)RAND_MAX) * 2 * range) - range;
		}
}

void printVector(float * vector, int N){
	for (int i = 0; i<N;i++){
			printf("%0.2f ", vector[i]);
	}
	printf("\n\n");
}

void printMatrix(float * matrix, int N){
	for (int i = 0; i<N;i++){
		for (int j = 0; j < N; j++){
			printf("%0.2f ", matrix[index(i,j,N)]);
		}
		printf("\n");
	}
	printf("\n");
}

int main(int argc, char *argv[])
{ 
	int i,j,N;
	
	if(argc < 2){
		printf("Not enough arguments, defaulting to N=%d. Usage: [N] [(optional) data_filename]\n",VSIZE);
		N = VSIZE;
	}else{
		//get size of vector
		N = atoi(argv[1]);
	}
	
	srand(time(NULL)); //seed the RNG
	//srand(1);
	
	//initialize memory 
	float * matA;
	float * vecB;
	float * vecC;
	
	matA = malloc(N*N*sizeof(float));
	vecB = malloc(N*sizeof(float));
	vecC = calloc(N,sizeof(float));
	
	//randomize A and B
	randomizeMatrix(R_CONST,N,matA);
	randomizeVector(R_CONST,N,vecB);
	
	//get start time 
	double time;
	time = omp_get_wtime();
	
	//do matrix-vector product
	for (i = 0; i < N; i++){
		for (j = 0; j < N; j++)
				vecC[i] += matA[index(i,j,N)]*vecB[j];
	}
	
	//calculate elapsed time for dot product
	time = omp_get_wtime() - time;

	printMatrix(matA, N);
	printVector(vecB, N);
	printVector(vecC,N);

	
	//calculate metrics 
	long FLOP = 2 * pow(N,2) - N; 
	double Flops = FLOP/time;
	
	#ifdef VERBOSE //comment out the #define above if you do not want this to print 
	printf("Performed a %d x %d - %d x 1 matrix-vector product in %f seconds\n", N, N, N,time);
	printf("Number of floating point operations = 2 * %d^2 - %d = %ld\n", N,N, FLOP);
	printf("Flops = %.3e\n", Flops);
	#endif
	
	//(optional) save metrics to file if argv[2] exists
	if(argc > 2){ //assume (in production code, check) 2nd arg is datafile out 
		FILE * csv_file;
		
		//first check if the file has already been created
		csv_file = fopen(argv[2],"r");
		if(!csv_file){
			//not created, reopen file as write, write column headers
			csv_file = fopen(argv[2],"w");
			fprintf(csv_file, "N,Flops,s\n");
		}
		fclose(csv_file); //close outside brackets in case the fopen in read worked
		
		// file is created from here onward, open in append mode and add the data 
		csv_file = fopen(argv[2],"a");
		fprintf(csv_file, "%d,%.3e,%f\n",N,FLOP,Flops,time);
		fclose(csv_file);
	}
	
	//free memory
	free(matA);
	free(vecB);
	free(vecC);
	
	return 0;
	
}
