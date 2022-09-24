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

void randomizeVector(float range, int N, float * vec){
	//uniform randomly initialize values in array between -range and +range
	for(int i = 0; i < N; i++)
		vec[i] = (((float)rand()/(float)RAND_MAX) * 2 * range) - range;
}

int main(int argc, char *argv[])
{ 
	int i,N;
	
	if(argc < 2){
		printf("Not enough arguments, defaulting to N=%d. Usage: [N] [(optional) data_filename]\n",MSIZE);
		N = MSIZE;
	}else{
		//get size of matrix 
		N = atoi(argv[1]);
	}
	
	srand(time(NULL)); //seed the RNG
	
	//initialize memory 
	float * vecy;
	float * vecx;
	float * vecb;
	
	vecy = malloc(N*  sizeof(float));
	vecx = malloc(N*  sizeof(float));
	vecb = calloc(N  ,sizeof(float));
	
	//randomize A and B
	randomizeVector(R_CONST,N,vecy);
	randomizeVector(R_CONST,N,vecx);
	
	//get start time 
	double time;
	time = omp_get_wtime();
	
	//do dot-product
	for (i = 0; i < N; i++){
		vecb[i] += vecy[i]*vecx[i];
	}
	
	//calculate elapsed time for matrix multiply
	time = omp_get_wtime() - time;
	
	//calculate metrics 
	long FLOP = N - 1; 
	double Flops = FLOP/time;
	
	#ifdef VERBOSE //by default not included 
	printf("Performed a %d dot-product in %f seconds\n", N, N, time);
	printf("Number of floating point operations = 2 * %d^3 = %ld\n", N, FLOP);
	printf("MFlops = %.2f\n", Flops/pow(10,6));
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
		fprintf(csv_file, "%d,%e,%lf\n",N,FLOP,Flops,time);
		fclose(csv_file);
	}
	
	//free memory
	free(vecy);
	free(vecx);
	free(vecb);
	
	return 0;
	
}
