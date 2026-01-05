import sys
import pandas as pd
import matplotlib.pyplot as plt

csv = sys.argv[1]
df = pd.read_csv(csv)

# ----------------------------------------------------
# PREPROCESSING — Average repeated runs
# ----------------------------------------------------
df = df.groupby(["N", "sparsity", "iterations", "threads"], as_index=False).mean()

# ----------------------------------------------------
# Compute derived metrics
# ----------------------------------------------------
df["serial_CSR_total"]    = df["serial_CSR_formation"]   + df["serial_CSR_mult"]
df["parallel_CSR_total"] = df["parallel_CSR_formation"] + df["parallel_CSR_mult"]

# ====================================================
# GLOBAL AVERAGE PLOTS
# ====================================================

# ----------------------------------------------------
# 1) GLOBAL: Sparsity vs Time
# ----------------------------------------------------
sparsity_avg = df.groupby("sparsity", as_index=False).mean()

plt.figure()
plt.plot(sparsity_avg["sparsity"], sparsity_avg["serial_CSR_total"], marker="o", label="CSR Total (Serial)")
plt.plot(sparsity_avg["sparsity"], sparsity_avg["parallel_CSR_total"], marker="o", label="CSR Total (Parallel)")
plt.plot(sparsity_avg["sparsity"], sparsity_avg["serial_dense_mult"], marker="o", label="Dense (Serial)")
plt.plot(sparsity_avg["sparsity"], sparsity_avg["parallel_dense_mult"], marker="o", label="Dense (Parallel)")
plt.xlabel("Sparsity (%)")
plt.ylabel("Avg Execution Time (s)")
plt.title("GLOBAL: Sparsity vs Time")
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.savefig("results/plot_sparsity_global_av2.png")
plt.close()

# ----------------------------------------------------
# 2) GLOBAL: Threads vs Time
# ----------------------------------------------------
threads_avg = df.groupby("threads", as_index=False).mean()

plt.figure()
plt.plot(threads_avg["threads"], threads_avg["serial_CSR_total"], marker="o", label="CSR Total (Serial)")
plt.plot(threads_avg["threads"], threads_avg["parallel_CSR_total"], marker="o", label="CSR Total (Parallel)")
plt.plot(threads_avg["threads"], threads_avg["serial_dense_mult"], marker="o", label="Dense (Serial)")
plt.plot(threads_avg["threads"], threads_avg["parallel_dense_mult"], marker="o", label="Dense (Parallel)")
plt.xlabel("Threads")
plt.ylabel("Avg Execution Time (s)")
plt.title("GLOBAL: Threads vs Time")
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.savefig("results/plot_threads_global_avg2.png")
plt.close()

# ----------------------------------------------------
# 3) GLOBAL: Iterations vs Time
# ----------------------------------------------------
iterations_avg = df.groupby("iterations", as_index=False).mean()

plt.figure()
plt.plot(iterations_avg["iterations"], iterations_avg["serial_CSR_total"], marker="o", label="CSR Total (Serial)")
plt.plot(iterations_avg["iterations"], iterations_avg["parallel_CSR_total"], marker="o", label="CSR Total (Parallel)")
plt.plot(iterations_avg["iterations"], iterations_avg["serial_dense_mult"], marker="o", label="Dense (Serial)")
plt.plot(iterations_avg["iterations"], iterations_avg["parallel_dense_mult"], marker="o", label="Dense (Parallel)")
plt.xlabel("Iterations")
plt.ylabel("Avg Execution Time (s)")
plt.title("GLOBAL: Iterations vs Time")
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.savefig("results/plot_iterations_global_avg2.png")
plt.close()

# ----------------------------------------------------
# 4) GLOBAL: N vs Time
# ----------------------------------------------------
N_avg = df.groupby("N", as_index=False).mean()

plt.figure()
plt.plot(N_avg["N"], N_avg["serial_CSR_total"], marker="o", label="CSR Total (Serial)")
plt.plot(N_avg["N"], N_avg["parallel_CSR_total"], marker="o", label="CSR Total (Parallel)")
plt.plot(N_avg["N"], N_avg["serial_dense_mult"], marker="o", label="Dense (Serial)")
plt.plot(N_avg["N"], N_avg["parallel_dense_mult"], marker="o", label="Dense (Parallel)")
plt.xlabel("Matrix Size N")
plt.ylabel("Avg Execution Time (s)")
plt.title("GLOBAL: N vs Time")
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.savefig("results/plot_N_global_avg2.png")
plt.close()

print("Created 4 global-avg plots in results/")
