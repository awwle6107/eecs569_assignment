#!/bin/bash

#SBATCH --job-name=mpi_dot_productr  # Job name
#SBATCH --partition=quickq      # Partition/Queue
#SBATCH --nodes=2               # Number of nodes
#SBATCH --ntasks-per-node=32     # CPUs per node (MAX=40 for compute nodes, 80 for GPU, 72 for quickq)
#SBATCH --time=12:00:00          # Maximum walltime of 1 hour (can ask for 12 hours for quickq if needed)
#SBATCH --output=mpi_dot_product-out-%j-%N.log  # Standard output (log file)

#everything under here is as if you typed it into your command line, but with the dedicated resources asked for above

# load any modules you need here
module load openmpi
module list

# run your program(s) here 

for P in 1 2 4 8 16 32
do
    for i in {1..5}
    do
        mpirun -np $P mpi_dot_product 4096 mpi_dot_product.csv
    done
done

for i in {1..20}
do
    mpirun -np 64 mpi_dot_product 4096 mpi_dot_product.csv
done