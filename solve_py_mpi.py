from mpi4py import MPI
import numpy as np
import sys

comm = MPI.COMM_WORLD
rank = comm.Get_rank()

if len(sys.argv) != 4:
    if rank == 0:
        print("Usage: python solve_py_mpi_auto.py A.txt b.txt out.txt")
    sys.exit(1)

A_file = sys.argv[1]
b_file = sys.argv[2]
out_file = sys.argv[3]

A = np.loadtxt(A_file)
b = np.loadtxt(b_file)
n = A.shape[0]
x = np.zeros(n, dtype=np.float64)

# Определяем тип матрицы
if np.allclose(A, np.tril(A)):
    tri_type = "lower"
elif np.allclose(A, np.triu(A)):
    tri_type = "upper"
else:
    if rank == 0:
        print("Error: matrix is not triangular")
    sys.exit(1)

def solve_lower(A, b):
    x_local = np.zeros_like(b)
    for i in range(n):
        total_sum = np.float64(0.0)
        for j in range(i):
            total_sum += np.float64(A[i,j]) * np.float64(x_local[j])
        if abs(A[i,i]) < 1e-12:
            raise ZeroDivisionError(f"Diagonal element too small at row {i}")
        x_local[i] = (np.float64(b[i]) - total_sum) / np.float64(A[i,i])
    return x_local

def solve_upper(A, b):
    x_local = np.zeros_like(b)
    for i in range(n-1, -1, -1):
        total_sum = np.float64(0.0)
        for j in range(i+1, n):
            total_sum += np.float64(A[i,j]) * np.float64(x_local[j])
        if abs(A[i,i]) < 1e-12:
            raise ZeroDivisionError(f"Diagonal element too small at row {i}")
        x_local[i] = (np.float64(b[i]) - total_sum) / np.float64(A[i,i])
    return x_local

if tri_type == "lower":
    x = solve_lower(A, b)
else:
    x = solve_upper(A, b)

if rank == 0:
    np.savetxt(out_file, x, fmt="%.8f")
    print(f"Matrix type detected: {tri_type}")
    print("Solution x:")
    print(" ".join([f"{xi:.8f}" for xi in x]))
