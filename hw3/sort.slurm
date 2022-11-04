#!/bin/bash

#SBATCH --job-name=mpisort
#SBATCH --partition=quickq
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=32
#SBATCH --time=8:00:00
#SBATCH --output=mpisort-out-%j-%N.log

module load openmpi
module list


for P in 2 4 8 16 32
do
    for i in {1..5}
    do
        mpirun -np $P mpi_sort 2147483648 mpi_sort.csv
    done
done