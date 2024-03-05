g++ -fopenmp main.cpp

for (( j=1; j<=18; j++ ))
do
    OMP_NUM_THREADS=$j OMP_NESTED=true ./a.out $j
done

