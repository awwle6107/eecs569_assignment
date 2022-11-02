#!/bin/bash

#SBATCH --job-name=prime_mpi  # Job name
#SBATCH --partition=quickq      # Partition/Queue
#SBATCH --nodes=2               # Number of nodes
#SBATCH --ntasks-per-node=32     # CPUs per node (MAX=40 for compute nodes, 80 for GPU, 72 for quickq)
#SBATCH --time=12:00:00          # Maximum walltime of 1 hour (can ask for 12 hours for quickq if needed)
#SBATCH --output=prime_mpi-out-%j-%N.log  # Standard output (log file)

#everything under here is as if you typed it into your command line, but with the dedicated resources asked for above

# load any modules you need here
module load openmpi
module list

# run your program(s) here 
# 1 thread
for i in {1..20}
do
    mpirun -np 1 prime_mpi 1048576 prime_mpi.csv
done

# 32 threads
for i in {1..20}
do
    mpirun -np 32 prime_mpi 1048576 prime_mpi.csv
done

# 64 threads
for i in {1..20}
do
    mpirun -np 64 prime_mpi 1048576 prime_mpi.csv
done