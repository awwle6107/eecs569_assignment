#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <mpi.h>

#define ROOT 0
#define R_CONST 100 

int main(int argc, char *argv[])
{ 

	if(argc < 3){
		//argc[1] is the filename from commandline
		printf("error: need at least two arguments. Usage: file_write [filename] [size] [optional: csv data filename]\n");
		exit(-1);
	}
	double t0;
	//initialize MPI and get size/rank
	int mpi_rank, mpi_size;
	
	MPI_Init(&argc,&argv);
	
	MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

	//get size of file as number of floats
	long total_size = atol(argv[2]);
	printf("thread %d is writing %ld floats to %s\n",mpi_rank,total_size,argv[1]);
	
	//allocate memory for size floats
	float * array = malloc(size * sizeof(float));
	
	//uniform randomly initialize values in array between -R_CONST and +R_CONST 
	srand(time(NULL)); //seed the RNG
	for(long i = 0; i < size; i++){
		array[i] = (((float)rand()/(float)RAND_MAX) * 2 * R_CONST) - R_CONST;
	}
	
	MPI_Barrier(MPI_COMM_WORLD);
	if(mpi_rank == ROOT){
		t0 = MPI_Wtime();
	}

	//open file for write 
	MPI_FILE fs;
	MPI_File_open(MPI_COMM_WORLD,argv[1],MPI_MODE_WRONLY|MPI_MODE_CREATE, MPI_INFO_NULL, &fs);	
	//time write, calculate performance
	double t = omp_get_wtime();
	fwrite(array,sizeof(float),size,arr_file);
	t = omp_get_wtime()-t;
	double MB = sizeof(float) * size / pow(10,6); //alternatively can use 2^20
	
	printf("Wrote %f MB in %f seconds at a rate of %f MB/s\n", MB, t, MB/t);
	
	//free array, close file 
	free(array);
	fclose(arr_file);
	
	//write data to a csv file if argv[3] exists
	if(argc > 3){ //assume (in production code, check) 3rd arg is datafile out 
		FILE * csv_file; //could reuse arr_file as it is closed, but more readable this way
		
		//first check if the file has already been created
		csv_file = fopen(argv[3],"r");
		if(!csv_file){
			//not created, reopen file as write, write column headers
			csv_file = fopen(argv[3],"w");
			fprintf(csv_file, "N,MB,s,MB/s\n");
		}
		fclose(csv_file); //close outside brackets in case the fopen in read worked
		
		// file is created from here onward, open in append mode and add the data 
		csv_file = fopen(argv[3],"a");
		fprintf(csv_file, "%d,%f,%f,%f\n",size,MB,t,MB/t);
		fclose(csv_file);
	}

	
	return 0;
	
}

