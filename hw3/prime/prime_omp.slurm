#!/bin/bash

#SBATCH --job-name=prime_omp  # Job name
#SBATCH --partition=quickq      # Partition/Queue
#SBATCH --nodes=1               # Number of nodes
#SBATCH --ntasks-per-node=32     # CPUs per node (MAX=40 for compute nodes, 80 for GPU, 72 for quickq)
#SBATCH --time=12:00:00          # Maximum walltime of 1 hour (can ask for 12 hours for quickq if needed)
#SBATCH --output=prime_omp-out-%j-%N.log  # Standard output (log file)

#everything under here is as if you typed it into your command line, but with the dedicated resources asked for above

# load any modules you need here
module list

# run your program(s) here 
export OMP_NUM_THREADS=1

for j in {1..20}
do
    ./prime_omp 1048576 prime_omp.csv
done

export OMP_NUM_THREADS=32

for j in {1..20}
do
    ./prime_omp 1048576 prime_omp.csv
done
