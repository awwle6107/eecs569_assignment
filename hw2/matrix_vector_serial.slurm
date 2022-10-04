#!/bin/bash

#SBATCH --job-name=mat_vect  # Job name
#SBATCH --partition=quickq      # Partition/Queue
#SBATCH --nodes=1               # Number of nodes
#SBATCH --ntasks-per-node=32     # CPUs per node (MAX=40 for compute nodes, 80 for GPU, 72 for quickq)
#SBATCH --time=12:00:00          # Maximum walltime of 1 hour (can ask for 12 hours for quickq if needed)
#SBATCH --output=out-%j-%N-mat_vect.log  # Standard output (log file)

#everything under here is as if you typed it into your command line, but with the dedicated resources asked for above

# load any modules you need here
module list

# run your program(s) here 

for T in 1 2 4 8 16
do
    export OMP_NUM_THREADS=$T
    for i in {1..5}
    do
        ./matrix_vector_serial 32768 matrix_vector_serial.csv
    done
done    

export OMP_NUM_THREADS=32

for j in {1..20}
do
    ./matrix_vector_serial 32768 matrix_vector_serial.csv
done
