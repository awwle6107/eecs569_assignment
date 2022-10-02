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
#define TSIZE 64

//range of random numbers
#define R_CONST 100 

//macro to index into matrix at location (i,j)
#define index(i,j,rowsize) i*rowsize+j

int main(int argc, char *argv[])
{ 
	int i,j,k,N,T;
	
	if(argc < 3){
		printf("Not enough arguments, defaulting to N=%d. Usage: [N] [T] [(optional) data_filename]\n",MSIZE);
		N = MSIZE;
		T = TSIZE;
	}else{
		//get size of matrix 
		N = atoi(argv[1]);
		T = atoi(argv[2]);
	}
	
	srand(time(NULL)); //seed the RNG
	
	//initialize memory 
	float matA[N][N];
	float matB[N][N];
	float matC[N][N];
	
	memset(matC, 0, sizeof(matC));
	
	//randomize A and B
	for(int i = 0; i < N; i++)
		for(int j = 0; j < N; j++){
			matA[i][j] = (((float)rand()/(float)RAND_MAX) * 2 * R_CONST) - R_CONST;
			matB[i][j] = (((float)rand()/(float)RAND_MAX) * 2 * R_CONST) - R_CONST;
		}
	
	//get start time 
	double time;
	time = omp_get_wtime();

	//do matrix multiply
	for(int ii = 0; ii < N; ii += T){
		for(int kk = 0; kk < N; kk += T){
			for(int jj = 0; jj < N; jj += T){
				for(int i = ii; i < ii + T; i++){
					for(int k = kk; k < kk + T; k++){
						for(int j = jj; j < jj + T; j++){
							matC[i][j] += matA[i][k]*matB[k][j];
						}
					}
				}
			}
		}
	}
	
	//calculate elapsed time for matrix multiply
	time = omp_get_wtime() - time;
	
	//calculate metrics 
	long FLOP = 2 * pow(N,3); 
	double Flops = FLOP/time;
	
	#ifdef VERBOSE //by default not included 
	printf("Performed a %d x %d matrix multiply in %f seconds\n", N, N, time);
	printf("Number of floating point operations = 2 * %d^3 = %ld\n", N, FLOP);
	printf("MFlops = %.2f\n", Flops/pow(10,6));
	#endif
	
	//(optional) save metrics to file if argv[2] exists
	if(argc > 3){ //assume (in production code, check) 2nd arg is datafile out 
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
		fprintf(csv_file, "%d,%e,%lf\n",N,FLOP,Flops,time);
		fclose(csv_file);
	}
	
	
	return 0;
	
}
