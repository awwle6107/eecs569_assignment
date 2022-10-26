#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>


int main(int argc, char *argv[])
{ 
	if(argc < 3){
		//argc[1] is the filename from commandline
		printf("error: need at least two arguments. Usage: file_read [filename] [size] [optional: csv data filename]\n");
		exit(-1);
	}
	
	//get size of file as number of floats
	long size = atol(argv[2]);
	printf("reading %ld floats from %s\n",size,argv[1]);
	
	//allocate memory for size floats
	float * array = malloc(size * sizeof(float));
	
	//open file for read 
	FILE * arr_file;
	arr_file = fopen(argv[1],"rb");
	
	//time read, calculate performance
	double t = omp_get_wtime();
	fread(array,sizeof(float),size,arr_file);
	t = omp_get_wtime()-t;
	double MB = sizeof(float) * size / pow(10,6); //alternatively can use 2^20
	
	printf("Read %f MB in %f seconds at a rate of %f MB/s\n", MB, t, MB/t);
	
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
