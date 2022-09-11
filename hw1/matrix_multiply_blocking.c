#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <omp.h>

//comment to silence statistics of performance 
#define VERBOSE

//default size of matrices are MSIZE x MSIZE
#define MSIZE 4096

//range of random numbers
#define R_CONST 100 

//macro to index into matrix at location (i,j)
#define index(i,j,rowsize) i*rowsize+j

//seed for random
#define SEED 10

void randomizeMatrix(float range, int N, float * matrix){
	//uniform randomly initialize values in array between -range and +range
	for(int i = 0; i < N; i++)
		for(int j = 0; j < N; j++){
			matrix[index(i,j,N)] = (((float)rand()/(float)RAND_MAX) * 2 * range) - range;
		}
}

void blocking_multiplication(float *matA, float *matB , float *matC, int N, int B)
{
    int i, j, k, ii, jj, kk;

    for (ii = 0; ii < N; ii += B)
    {
        for (jj = 0; jj < N; jj += B)
        {
            for (kk = 0; kk < N; kk += B)
            {
                for (i = ii; i < ii + B; i++)
                {
                    for (j = jj; j < jj + B; j++)
                    {
                        for (k = kk; k < kk + B; k++)
                        {
                            matC[index(i, j, N)] += matA[index(i, k, N)] * matB[index(k, j, N)];
                        }
                    }
                }
            }
        }
    }
}

void print_matrix(float * matrix, int N){
	for (int i = 0; i < N; i++){
		for (int j = 0; j < N; j++){
			printf("%0.2f ", matrix[index(i, j, N)]);
		}
		printf("\n");
	}
	printf("\n");
}

int main(int argc, char *argv[])
{ 
	int i,j,k,N,block;
	
	if(argc < 3){
		printf("Not enough arguments, defaulting to N=%d. Usage: [N] [(optional) data_filename]\n",MSIZE);
		N = MSIZE;
	}else{
		//get size of matrix 
		N = atoi(argv[1]);
        block = atoi(argv[2]);
	}
	
	srand(SEED); //seed the RNG
	
	//initialize memory 
	float * matA;
	float * matB;
	float * matC;
	matA = malloc(N*N*sizeof(float));
	matB = malloc(N*N*sizeof(float));
	matC = calloc(N*N,sizeof(float));
	//randomize A and B
	randomizeMatrix(R_CONST,N,matA);
	randomizeMatrix(R_CONST,N,matB);
	
	
	//get start time 
	double time;
	time = omp_get_wtime();
	
	//do matrix multiply
	// for (i = 0; i < N; i++){
	// 	for (j = 0; j < N; j++){
	// 		float val = 0;
	// 		for (k = 0; k < N; k++)
	// 			val += matA[index(i,k,N)]*matB[index(k,j,N)];
	// 		matC[index(i,j,N)] = val;
	// 	}
	// }
    blocking_multiplication(matA, matB, matC, N, block);
	
	//calculate elapsed time for matrix multiply
	time = omp_get_wtime() - time;
	
	//calculate metrics 
	long FLOP = 2 * pow(N,3); 
	double Flops = FLOP/time;
	
	#ifdef VERBOSE //comment out the #define above if you do not want this to print 
	printf("Performed a %d x %d matrix multiply in %f seconds\n", N, N, time);
	printf("Number of floating point operations = 2 * %d^3 = %ld\n", N, FLOP);
	printf("Flops = %.3e\n", Flops);
	#endif
	
	//(optional) save metrics to file if argv[2] exists
	if(argc > 2){ //assume (in production code, check) 2nd arg is datafile out 
		FILE * csv_file;
		
		//first check if the file has already been created
		csv_file = fopen(argv[3],"r");
		if(!csv_file){
			//not created, reopen file as write, write column headers
			csv_file = fopen(argv[3],"w");
			fprintf(csv_file, "N,Flops,s\n");
		}
		fclose(csv_file); //close outside brackets in case the fopen in read worked
		
		// file is created from here onward, open in append mode and add the data 
		csv_file = fopen(argv[3],"a");
		fprintf(csv_file, "%d,%.3e,%f\n",N,FLOP,Flops,time);
		fclose(csv_file);
	}
	
	//free memory
	free(matA);
	free(matB);
	free(matC);
	
	return 0;
	
}
