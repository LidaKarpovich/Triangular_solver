
#!/bin/bash
#SBATCH --job-name=tri_solve
#SBATCH --nodes=3
#SBATCH --ntasks=112
#SBATCH --cpus-per-task=1
#SBATCH --time=01:00:00
#SBATCH --output=run_scaling_%j.out

module load compiler/gcc/11.2.0 mpi/openmpi/4.1.6/gcc/11 python/3.11

export OMPI_MCA_btl="tcp,self"

# Переменные
A_file="test_A.txt"
b_file="test_b.txt"
tol=1e-5

# Функция запуска C+OpenMP
run_openmp() {
  for n in {1..48}; do
    export OMP_NUM_THREADS=$n
    out_file="out_C_OpenMP_${n}.txt"
    ./solve_omp $A_file $b_file $out_file
  done
}

# Функция запуска C+pthreads
run_pthreads() {
  for n in {1..48}; do
    export OMP_NUM_THREADS=$n 
    out_file="out_C_pthreads_${n}.txt"
    ./solve_pthreads $A_file $b_file $out_file
  done
}

# Функция запуска C+MPI
run_c_mpi() {
  for n in {1..112}; do
    out_file="out_C_MPI_${n}.txt"
    mpirun -np $n ./solve_mpi $A_file $b_file $out_file
  done
}

# Функция запуска Python+MPI
run_py_mpi() {
  for n in {1..112}; do
    out_file="out_Python_MPI_${n}.txt"
    mpirun -np $n python3 ./solve_py_mpi.py $A_file $b_file $out_file
  done
}

# Запуск всех реализаций
echo "Running C+OpenMP..."
run_openmp
echo "Running C+pthreads..."
run_pthreads
echo "Running C+MPI..."
run_c_mpi
echo "Running Python+MPI..."
run_py_mpi

echo "All runs finished!"

python3 check_and_save_csv.py
