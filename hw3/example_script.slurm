#!/bin/bash

#SBATCH --job-name=eecs592_hw3_mpibmark  # Job name
#SBATCH --nodes=4                        # Number of nodes
#SBATCH --ntasks-per-node=32             # Number of processes
#SBATCH --output=out-%j-%N.log  # Standard output (log file)
#SBATCH --partition=compute     # Partition/Queue
#SBATCH --time=0:20:00          # Maximum walltime of 20 minutes

module purge
module use /cm/shared/modulefiles_local
module load shared
module load slurm
module load gcc
module load openmpi
module list

mpirun -np 128 IMB-MPI1 bcast scatter gather reduce allreduce allgather barrier