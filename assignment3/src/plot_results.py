#!/usr/bin/env python3
import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path

RESULTS = Path("results/raw_results.csv")
OUTDIR  = Path("results/plots")
OUTDIR.mkdir(parents=True, exist_ok=True)

df = pd.read_csv(RESULTS)

# Average over repeated runs
df_avg = df.groupby(["N","sparsity","iterations","processes"]).mean().reset_index()

# ------------------------------
# 1️⃣ Total time vs iterations
# ------------------------------
plt.figure(figsize=(8,6))
sub = df_avg.groupby("iterations")[[
    "Total_serial_CSR","Total_parallel_CSR",
    "Total_serial_dense","Total_parallel_dense"
]].mean()

sub.plot(marker='o', ax=plt.gca())
plt.xlabel("Iterations")
plt.ylabel("Time (s)")
plt.title("Total Time vs Iterations (Global)")
plt.grid(True)
plt.legend(["Serial CSR","Parallel CSR","Serial Dense","Parallel Dense"])
plt.savefig(OUTDIR / "total_vs_iterations.png")
plt.close()

# ------------------------------
# 2️⃣ Total time vs sparsity
# ------------------------------
plt.figure(figsize=(8,6))
sub = df_avg.groupby("sparsity")[[
    "Total_serial_CSR","Total_parallel_CSR",
    "Total_serial_dense","Total_parallel_dense"
]].mean()

sub.plot(marker='o', ax=plt.gca())
plt.xlabel("Sparsity (%)")
plt.ylabel("Time (s)")
plt.title("Total Time vs Sparsity (Global)")
plt.grid(True)
plt.legend(["Serial CSR","Parallel CSR","Serial Dense","Parallel Dense"])
plt.savefig(OUTDIR / "total_vs_sparsity.png")
plt.close()

# ------------------------------
# 3️⃣ Total time vs matrix size
# ------------------------------
plt.figure(figsize=(8,6))
sub = df_avg.groupby("N")[[
    "Total_serial_CSR","Total_parallel_CSR",
    "Total_serial_dense","Total_parallel_dense"
]].mean()

sub.plot(marker='o', ax=plt.gca())
plt.xlabel("Matrix Size N")
plt.ylabel("Time (s)")
plt.title("Total Time vs Matrix Size (Global)")
plt.grid(True)
plt.legend(["Serial CSR","Parallel CSR","Serial Dense","Parallel Dense"])
plt.savefig(OUTDIR / "total_vs_size.png")
plt.close()

# ------------------------------
# 4️⃣ Parallel CSR scaling breakdown
# ------------------------------
plt.figure(figsize=(8,6))
df_csr = df_avg.groupby("processes")[[
    "CSR_construction","CSR_msg_send","CSR_parallel","Total_parallel_CSR"
]].mean()

df_csr.plot(marker='o', ax=plt.gca())
plt.xlabel("Processes")
plt.ylabel("Time (s)")
plt.title("Parallel CSR: Breakdown vs Processes")
plt.grid(True)
plt.legend(["Construction","Message Send","Multiply","Total"])
plt.savefig(OUTDIR / "parallel_csr_breakdown.png")
plt.close()

# ------------------------------
# 5️⃣ Parallel Dense scaling breakdown
# ------------------------------
plt.figure(figsize=(8,6))
df_dense = df_avg.groupby("processes")[[
    "Dense_msg_send","Dense_parallel","Total_parallel_dense"
]].mean()

df_dense.plot(marker='o', ax=plt.gca())
plt.xlabel("Processes")
plt.ylabel("Time (s)")
plt.title("Parallel Dense: Breakdown vs Processes")
plt.grid(True)
plt.legend(["Message Send","Multiply","Total"])
plt.savefig(OUTDIR / "parallel_dense_breakdown.png")
plt.close()

print("✅ All 5 main plots saved in results/plots/")
