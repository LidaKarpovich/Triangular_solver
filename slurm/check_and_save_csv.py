import subprocess
import time
import csv
import os
import numpy as np

A_file = 'test_A.txt'
b_file = 'test_b.txt'
tol = 1e-5  # допустимая погрешность для float

# Эталонное решение numpy
A = np.loadtxt(A_file)
b = np.loadtxt(b_file)
x_true = np.linalg.solve(A, b)

executables = {
    'C+OpenMP': './solve_omp',
    'C+pthreads': './solve_pthreads',
    'C+MPI': './solve_mpi',
    'Python+MPI': 'python3 solve_py_mpi.py'
}

# Кол-во потоков/процессов для теста
threads_range = range(1, 49)   # OpenMP / pthreads
mpi_range = range(1, 113)      # MPI

# CSV для результатов
csv_file = 'scaling_results.csv'
with open(csv_file, 'w', newline='') as f:
    writer = csv.writer(f)
    writer.writerow(['Implementation', 'Threads/Processes', 'Time_s', 'PASS'])

    for name, exe in executables.items():
        print(f"\nRunning scaling test for {name}...")

        if 'MPI' in name:
            rng = mpi_range
        else:
            rng = threads_range

        for n in rng:
            out_file = f'out_{name.replace("+","_")}_{n}.txt'
            start_time = time.time()

            try:
                if name == 'C+OpenMP':
                    env = os.environ.copy()
                    env['OMP_NUM_THREADS'] = str(n)
                    subprocess.run([exe, A_file, b_file, out_file], env=env, ch$
                elif name == 'C+pthreads':
                    env = os.environ.copy()
                    env['OMP_NUM_THREADS'] = str(n)
                    subprocess.run([exe, A_file, b_file, out_file], env=env, ch$
                elif 'MPI' in name:
                    cmd = f"mpirun -np {n} {exe} {A_file} {b_file} {out_file}"
                    subprocess.run(cmd, shell=True, check=True)
                elapsed = time.time() - start_time

                # Проверка корректности решения
                x_computed = np.loadtxt(out_file)
                pass_check = np.allclose(x_computed, x_true, atol=tol)

                print(f"{name}, n={n}, time={elapsed:.4f}s, PASS={pass_check}")
                writer.writerow([name, n, elapsed, pass_check])

            except Exception as e:
                elapsed = time.time() - start_time
                print(f"{name}, n={n}, ERROR: {e}")
                writer.writerow([name, n, elapsed, False])
