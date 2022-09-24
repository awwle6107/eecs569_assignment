#!/bin/bash

#SBATCH --job-name=eecs569_hw2_sync # Job name
#SBATCH --nodes=1               	# Number of nodes
#SBATCH --ntasks-per-node=32    	# CPUs per node (MAX=40 for CPU nodes and 80 for GPU)
#SBATCH --output=out-%j-%N.log  	# Standard output (log file)
#SBATCH --partition=quickq     	# Partition/Queue
#SBATCH --time=0:05:00          	# Maximum walltime of 5 minutes

module purge
module use /cm/shared/modulefiles_local
module load shared
module load slurm
module load gcc/11.3.0
module list

export OMP_NUM_THREADS=32

echo $OMP_NUM_THREADS

date
./syncbench --outer-repetitions 50 --delay-time 0.01 --test-time 10000 > syncbench.csv
./schedbench --outer-repetitions 50 --delay-time 0.01 --test-time 10000 > schedbench.csv
date
