import sys
import pandas as pd
import matplotlib.pyplot as plt

csv = sys.argv[1]
df = pd.read_csv(csv)

# Pre-average: only average repeated runs
df = df.groupby(["N", "sparsity", "iterations", "threads"], as_index=False).mean()

# ------------------------------
# 1) Sparsity vs Time for each matrix size
# ------------------------------
matrix_sizes = sorted(df["N"].unique())

for N in matrix_sizes:
    # Fix N, average across all iterations & threads
    sub = df[df["N"] == N].groupby("sparsity", as_index=False).mean()

    plt.figure()
    plt.plot(sub["sparsity"], sub["CSR_total_time"], marker='o', label="CSR Total Time")
    plt.plot(sub["sparsity"], sub["Dense_time"], marker='o', label="Dense Time")

    plt.xlabel("Sparsity (%)")
    plt.ylabel("Avg Execution Time (s)")
    plt.title(f"Sparsity vs Execution Time — N={N}")
    plt.grid(True)
    plt.legend()
    plt.tight_layout()
    plt.savefig(f"results/plot_sparsity_N{N}.png")
    plt.close()

# ------------------------------
# 2) Iterations vs Time (N0, sparsity=70%)
# ------------------------------
N0 = matrix_sizes[0]
sub_70 = df[(df["N"] == N0) & (df["sparsity"] == 70)]

# Fix N, sparsity → average over threads
iter_curve = sub_70.groupby("iterations", as_index=False).mean()

plt.figure()
plt.plot(iter_curve["iterations"], iter_curve["CSR_total_time"], marker='o', label="CSR Total Time")
plt.plot(iter_curve["iterations"], iter_curve["Dense_time"], marker='o', label="Dense Time")

plt.xlabel("Iterations")
plt.ylabel("Execution Time (s)")
plt.title(f"Iterations vs Time — N={N0}, Sparsity=70%")
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.savefig(f"results/plot_iterations_N{N0}_s70.png")
plt.close()

# ------------------------------
# 3) Threads vs Time (N0, sparsity=70%)
# ------------------------------

# Fix N, sparsity → average over iterations
thread_curve = sub_70.groupby("threads", as_index=False).mean()

plt.figure()
plt.plot(thread_curve["threads"], thread_curve["CSR_total_time"], marker='o', label="CSR Total Time")
plt.plot(thread_curve["threads"], thread_curve["Dense_time"], marker='o', label="Dense Time")

plt.xlabel("Threads")
plt.ylabel("Execution Time (s)")
plt.title(f"Threads vs Time — N={N0}, Sparsity=70%")
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.savefig(f"results/plot_threads_N{N0}_s70.png")
plt.close()

print("Clean plots created in results/")
