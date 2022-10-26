#!/bin/bash

#SBATCH --job-name=n2p1_pp  # Job name
#SBATCH --nodes=2                        # Number of nodes
#SBATCH --ntasks-per-node=1             # Number of processes
#SBATCH --output=n2p1_pp_out-%j-%N.log  # Standard output (log file)
#SBATCH --partition=compute     # Partition/Queue
#SBATCH --time=12:00:00          # Maximum walltime of 20 minutes

module purge
module use /cm/shared/modulefiles_local
module load shared
module load slurm
module load gcc
module load openmpi
module list
mpirun -np 2 IMB-MPI1 pingpong
