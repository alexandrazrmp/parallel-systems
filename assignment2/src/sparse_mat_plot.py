import sys
import pandas as pd
import matplotlib.pyplot as plt

csv = sys.argv[1]
df = pd.read_csv(csv)

# ----------------------------------------------------
# PREPROCESSING — Average repeated runs
# ----------------------------------------------------
df = df.groupby(["N", "sparsity", "iterations", "threads"], as_index=False).mean()

# ====================================================
# GLOBAL AVERAGE PLOTS (Only these 4)
# ====================================================

# ----------------------------------------------------
# 1) GLOBAL: Sparsity vs Time
# ----------------------------------------------------
sparsity_avg = df.groupby("sparsity", as_index=False).mean()

plt.figure()
plt.plot(sparsity_avg["sparsity"], sparsity_avg["CSR_total_time"], marker="o", label="CSR Total")
plt.plot(sparsity_avg["sparsity"], sparsity_avg["CSR_formation_time"], marker="o", label="CSR Formation")
plt.plot(sparsity_avg["sparsity"], sparsity_avg["CSR_multiplication_time"], marker="o", label="CSR Multiplication")
plt.plot(sparsity_avg["sparsity"], sparsity_avg["Dense_time"], marker="o", label="Dense")
plt.xlabel("Sparsity (%)")
plt.ylabel("Avg Execution Time (s)")
plt.title("GLOBAL: Sparsity vs Time")
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.savefig("results/plot_sparsity_global_avg.png")
plt.close()

# ----------------------------------------------------
# 2) GLOBAL: Threads vs Time
# ----------------------------------------------------
threads_avg = df.groupby("threads", as_index=False).mean()

plt.figure()
plt.plot(threads_avg["threads"], threads_avg["CSR_total_time"], marker="o", label="CSR Total")
plt.plot(threads_avg["threads"], threads_avg["CSR_formation_time"], marker="o", label="CSR Formation")
plt.plot(threads_avg["threads"], threads_avg["CSR_multiplication_time"], marker="o", label="CSR Multiplication")
plt.plot(threads_avg["threads"], threads_avg["Dense_time"], marker="o", label="Dense")
plt.xlabel("Threads")
plt.ylabel("Avg Execution Time (s)")
plt.title("GLOBAL: Threads vs Time")
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.savefig("results/plot_threads_global_avg.png")
plt.close()

# ----------------------------------------------------
# 3) GLOBAL: Iterations vs Time
# ----------------------------------------------------
iterations_avg = df.groupby("iterations", as_index=False).mean()

plt.figure()
plt.plot(iterations_avg["iterations"], iterations_avg["CSR_total_time"], marker="o", label="CSR Total")
plt.plot(iterations_avg["iterations"], iterations_avg["CSR_formation_time"], marker="o", label="CSR Formation")
plt.plot(iterations_avg["iterations"], iterations_avg["CSR_multiplication_time"], marker="o", label="CSR Multiplication")
plt.plot(iterations_avg["iterations"], iterations_avg["Dense_time"], marker="o", label="Dense")
plt.xlabel("Iterations")
plt.ylabel("Avg Execution Time (s)")
plt.title("GLOBAL: Iterations vs Time")
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.savefig("results/plot_iterations_global_avg.png")
plt.close()

# ----------------------------------------------------
# 4) GLOBAL: N vs Time
# ----------------------------------------------------
N_avg = df.groupby("N", as_index=False).mean()

plt.figure()
plt.plot(N_avg["N"], N_avg["CSR_total_time"], marker="o", label="CSR Total")
plt.plot(N_avg["N"], N_avg["CSR_formation_time"], marker="o", label="CSR Formation")
plt.plot(N_avg["N"], N_avg["CSR_multiplication_time"], marker="o", label="CSR Multiplication")
plt.plot(N_avg["N"], N_avg["Dense_time"], marker="o", label="Dense")
plt.xlabel("Matrix Size N")
plt.ylabel("Avg Execution Time (s)")
plt.title("GLOBAL: N vs Time")
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.savefig("results/plot_N_global_avg.png")
plt.close()

print("Created 4 global-avg plots in results/")
