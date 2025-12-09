import numpy as np
import subprocess
import os

from mpi4py import MPI
comm = MPI.COMM_WORLD

# запуск тестера только на rank 0
if comm.rank != 0:
    exit(0)


def generate_triangular_matrix(n, kind):
    A = np.tril(np.random.rand(n,n) + 1.0)
    return A if kind == "lower" else np.triu(np.random.rand(n,n) + 1.0) 

def save_matrix_vector(A, b, prefix):
    np.savetxt(f"{prefix}_A.txt", A)
    np.savetxt(f"{prefix}_b.txt", b)

def load_vector(path):
    return np.loadtxt(path)

def run_exec(cmd, output_file):
    try:
        subprocess.run(cmd, check=True,
                       stdout=subprocess.PIPE,
                       stderr=subprocess.PIPE)
        return load_vector(output_file)
    except Exception as e:
        print("    ERROR:", e)
        return None


IMPLEMENTATIONS = [
    {
        "name": "pthread",
        "supports": ["lower", "upper"],
        "cmd": lambda A, b, out: ["./solve_pthreads", A, b, out]
    },
    {
        "name": "omp",
        "supports": ["lower", "upper"],
        "cmd": lambda A, b, out: ["./solve_omp", A, b, out]
    },
    {
        "name": "mpi_c",
        "supports": ["lower", "upper"],
        "cmd": lambda A, b, out: ["mpiexec", "-n", "4", "./solve_mpi", A, b, out]
    },
    {
        "name": "py_mpi",
        "supports": ["lower", "upper"],
        "cmd": lambda A, b, out: ["mpiexec", "-n", "4", "python",
                                  "solve_py_mpi.py", A, b, out]
    },
]


def run_tests():
    np.random.seed(42)

    sizes = [4, 10, 50]
    repeats = 2
    tolerance = 1e-5

    print("\n===== START TESTING =====\n")

    for n in sizes:
        for rep in range(repeats):
            for kind in ["lower", "upper"]:

                print(f"=== Test n={n}, matrix={kind}, repetition {rep+1} ===")

                A = generate_triangular_matrix(n, kind)
                b = np.random.rand(n)
                save_matrix_vector(A, b, "test")

                # эталон
                x_ref = np.linalg.solve(A, b)

                for impl in IMPLEMENTATIONS:

                    name = impl["name"]

                    if kind not in impl["supports"]:
                        print(f"  {name}: SKIPPED (does not support {kind})")
                        continue

                    out_file = f"out_{n}_{kind}_{rep}_{name}.txt"
                    cmd = impl["cmd"]("test_A.txt", "test_b.txt", out_file)

                    print(f"  {name}: running... ", end="")
                    x = run_exec(cmd, out_file)

                    if x is None:
                        print("ERROR")
                        continue

                    ok = np.allclose(x, x_ref, atol=tolerance)
                    print("PASS" if ok else "FAIL")

                    if not ok:
                        print("    ref:", x_ref)
                        print("    got:", x)

                print("-" * 40)

    print("\n===== TESTING COMPLETE =====")


if __name__ == "__main__":
    run_tests()
