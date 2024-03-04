#PBS -l walltime=00:09:59
#PBS -l nodes=7:ppn=4
#PBS -N wowJob
#PBS -q batch

cd $PBS_O_WORKDIR
export OMP_NUM_THREADS=$PBS_NUM_PPN


for i in $(seq 21 1 28)
do
        for j in $(seq 1 1 10)
        do
                mpirun --hostfile $PBS_NODEFILE -np $i ./life2d p46gun.cfg
        done
done
