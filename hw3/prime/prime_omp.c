#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <omp.h>
#define MSIZE 10000

int main(int argc, char *argv[])
{
    int i, j, k, N;
    int total = 0;
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
    double time;
	time = omp_get_wtime();
    #pragma omp parallel 
    {
        int i,j,prime;
        #pragma omp for reduction (+: total)
        for (i = 2; i <= N; i++)
        {
            prime = 1;
            for (j = 2; j < i; j++)
            {
                if ((i % j) == 0)
                {
                    prime = 0;
                    break;
                }
            }
            total = total + prime;
        }
    }
    printf("there are %d primes under %ld\n", total, N);

    time = omp_get_wtime() - time;
    char* OMP_NUM_THREADS = getenv("OMP_NUM_THREADS");
	printf("took %f seconds with OMP_NUM_THREADS: %s\n\n", time, OMP_NUM_THREADS );
    if(argc > 2){ //assume (in production code, check) 2nd arg is datafile out 
		FILE * csv_file;
		
		//first check if the file has already been created
		csv_file = fopen(argv[2],"r");
		if(!csv_file){
			//not created, reopen file as write, write column headers
			csv_file = fopen(argv[2],"w");
			fprintf(csv_file, "N,s,T\n");
		}
		fclose(csv_file); //close outside brackets in case the fopen in read worked
		
		// file is created from here onward, open in append mode and add the data 
		csv_file = fopen(argv[2],"a");
		fprintf(csv_file, "%d,%lf,%s\n",N,time,OMP_NUM_THREADS);
		fclose(csv_file);
	}
}