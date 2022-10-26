#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <mpi.h>

#define MSIZE 1024

void shuffle(int *array, long n);
int comparison(const void * a, const void * b);

int main(int argc, char * argv[]){
	long N; //size of array
	
	//get size of array from command line 
	if(argc < 2){
		printf("Not enough arguments, defaulting to N=%d. Usage: [N] [(optional) data_filename]\n",MSIZE);
		N = MSIZE;
	}else{
		//get size of matrix 
		N = atol(argv[1]);
	}
	
	//create array in order:
	int * arr = malloc(sizeof(int)*N);
	for(long i = 0; i < N; i++)
		arr[i] = i;
	
	//shuffle the array
	srand(time(NULL));
	shuffle(arr, N);
	printf("0: [%d], 10: [%d]\n",arr[0],arr[10]);
	
	double t = MPI_Wtime();
	//sort the array
	qsort(arr,N,sizeof(int),comparison);
	
	t = MPI_Wtime() - t;
	
	printf("sorted %d numbers in %f s\n",N,t);
	printf("0: [%d], 10: [%d]\n",arr[0],arr[10]);
	
	//TODO: add code to save the time (you can get this from the matrix algebra code)
	
	return 0;
}

/* Arrange the N elements of ARRAY in random order.
   Only effective if N is much smaller than RAND_MAX;
   if this may not be the case, use a better random
   number generator. */
   
//From: https://benpfaff.org/writings/clc/shuffle.html
void shuffle(int *array, long n)
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

//comparison function for qsort
//based on: https://www.tutorialspoint.com/c_standard_library/c_function_qsort.htm
int comparison(const void * a, const void * b){
	return ( *(int*)a - *(int*)b );
}