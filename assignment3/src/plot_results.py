#!/usr/bin/env python3

import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path

RESULTS = Path("results/raw_results.csv")
OUTDIR  = Path("results/plots")
OUTDIR.mkdir(parents=True, exist_ok=True)

df = pd.read_csv(RESULTS)

# Average over repeated runs
g = df.groupby(["N","sparsity","iterations","processes"]).mean().reset_index()

# ----------------------------------------------------
# 1️⃣ Strong scaling of CSR vs Serial
# ----------------------------------------------------
def plot_strong_scaling():
    for N in g["N"].unique():
        for sp in g["sparsity"].unique():
            for it in g["iterations"].unique():
                sub = g[(g.N==N) & (g.sparsity==sp) & (g.iterations==it)]
                if len(sub) == 0: continue

                plt.figure()
                plt.plot(sub["processes"], sub["Total_parallel_CSR"], marker="o", label="Parallel CSR")
                plt.plot(sub["processes"], sub["Total_serial_CSR"], marker="s", label="Serial CSR")
                plt.xlabel("Processes")
                plt.ylabel("Time (s)")
                plt.title(f"Strong Scaling | N={N}, sparsity={sp}%, iters={it}")
                plt.legend()
                plt.grid(True)
                plt.savefig(OUTDIR / f"strong_scaling_N{N}_sp{sp}_it{it}.png")
                plt.close()

# ----------------------------------------------------
# 2️⃣ CSR vs Dense
# ----------------------------------------------------
def plot_csr_vs_dense():
    for N in g["N"].unique():
        for sp in g["sparsity"].unique():
            for it in g["iterations"].unique():
                sub = g[(g.N==N) & (g.sparsity==sp) & (g.iterations==it)]
                if len(sub) == 0: continue

                plt.figure()
                plt.plot(sub["processes"], sub["Total_parallel_CSR"], marker="o", label="Parallel CSR")
                plt.plot(sub["processes"], sub["Parallel_dense"], marker="s", label="Parallel Dense")
                plt.xlabel("Processes")
                plt.ylabel("Time (s)")
                plt.title(f"CSR vs Dense | N={N}, sparsity={sp}%, iters={it}")
                plt.legend()
                plt.grid(True)
                plt.savefig(OUTDIR / f"csr_vs_dense_N{N}_sp{sp}_it{it}.png")
                plt.close()

# ----------------------------------------------------
# 3️⃣ Effect of sparsity on performance
# ----------------------------------------------------
def plot_sparsity_effect():
    for N in g["N"].unique():
        for P in g["processes"].unique():
            for it in g["iterations"].unique():
                sub = g[(g.N==N) & (g.processes==P) & (g.iterations==it)]
                if len(sub) == 0: continue

                plt.figure()
                plt.plot(sub["sparsity"], sub["Total_parallel_CSR"], marker="o", label="Parallel CSR")
                plt.plot(sub["sparsity"], sub["Parallel_dense"], marker="s", label="Parallel Dense")
                plt.xlabel("Sparsity (%)")
                plt.ylabel("Time (s)")
                plt.title(f"Sparsity Impact | N={N}, P={P}, iters={it}")
                plt.legend()
                plt.grid(True)
                plt.savefig(OUTDIR / f"sparsity_N{N}_P{P}_it{it}.png")
                plt.close()

# ----------------------------------------------------
# 4️⃣ Breakdown of CSR cost components
# ----------------------------------------------------
def plot_csr_breakdown():
    for N in g["N"].unique():
        for P in g["processes"].unique():
            for sp in g["sparsity"].unique():
                for it in g["iterations"].unique():
                    sub = g[(g.N==N) & (g.processes==P) & (g.sparsity==sp) & (g.iterations==it)]
                    if len(sub) == 0: continue

                    row = sub.iloc[0]
                    labels = ["Construction", "Message Send", "Computation"]
                    values = [row.CSR_construction, row.CSR_msg_send, row.CSR_parallel]

                    plt.figure()
                    plt.bar(labels, values)
                    plt.ylabel("Time (s)")
                    plt.title(f"CSR Breakdown | N={N}, P={P}, sp={sp}%, iters={it}")
                    plt.savefig(OUTDIR / f"csr_breakdown_N{N}_P{P}_sp{sp}_it{it}.png")
                    plt.close()

# ----------------------------------------------------
# Run everything
# ----------------------------------------------------
plot_strong_scaling()
plot_csr_vs_dense()
plot_sparsity_effect()
plot_csr_breakdown()

print("Plots saved in results/plots/")
