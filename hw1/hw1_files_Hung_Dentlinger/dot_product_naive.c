#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <omp.h>

//MAIN CHANGES: changes all references of matrix to vector, all index values were changed to [i]

//comment to silence statistics of performance 
#define VERBOSE

//default size of vectors are VSIZE
#define VSIZE 4096

//range of random numbers
#define R_CONST 100

//macro to index into vector at location (i,j)  - didn't need for vectors
//#define index(i,j,rowsize) i*rowsize+j

// randomize was changed by removing the second for loop
void randomizeVector(float range, int N, float * vector){
	//uniform randomly initialize values in array between -range and +range
	for(int i = 0; i < N; i++)
		vector[i] = (((float)rand()/(float)RAND_MAX) * 2 * range) - range;
}

void printVector(float * vector, int N){
	for (int i = 0; i<N;i++){
			printf("%0.2f ", vector[i]);
	}
	printf("\n");
}

int main(int argc, char *argv[])
{ 
	int i,N;
	
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
	float * vecA;
	float * vecB;
	float C = 0.0;
	
	//removed an N for each of the vector mallocs
	vecA = malloc(N*sizeof(float));
	vecB = malloc(N*sizeof(float));
	//matC = calloc(N*N,sizeof(float));
	
	//randomize A and B
	randomizeVector(R_CONST,N,vecA);
	randomizeVector(R_CONST,N,vecB);
	
	//get start time 
	double time;
	time = omp_get_wtime();
	
	//dot product removed the two inner for loops, just needed the single outer loop, C is updated each cycle

	//do dot product
	for (i = 0; i < N; i++){
				C += vecA[i]*vecB[i];
	}
	
	//calculate elapsed time for dot product
	time = omp_get_wtime() - time;

	//printVector(vecA, N);
	//printVector(vecB, N);

	
	//calculate metrics ---- FLOP equation updated to 2*N-1
	long FLOP = 2*N-1; 
	double Flops = FLOP/time;
	
	#ifdef VERBOSE //comment out the #define above if you do not want this to print 
	printf("Performed a %d x 1 dot product in %f seconds\n", N, time);
	printf("Number of floating point operations = 2 * %d - 1 = %ld\n", N, FLOP);
	printf("Flops = %.3e\n", Flops);
	//printf("C = %0.3f\n",C);
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
	free(vecA);
	free(vecB);
	//free(C);
	
	return 0;
	
}
