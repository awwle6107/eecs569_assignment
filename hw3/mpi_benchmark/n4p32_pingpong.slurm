#!/bin/bash

#SBATCH --job-name=n4p32_pp  # Job name
#SBATCH --nodes=4                        # Number of nodes
#SBATCH --ntasks-per-node=32             # Number of processes
#SBATCH --output=n4p32_pp_out-%j-%N.log  # Standard output (log file)
#SBATCH --partition=compute     # Partition/Queue
#SBATCH --time=12:00:00          # Maximum walltime of 20 minutes

module purge
module use /cm/shared/modulefiles_local
module load shared
module load slurm
module load gcc
module load openmpi
module list
mpirun -np 128 IMB-MPI1 bcast scatter gather reduce allreduce allgather barrier alltoall