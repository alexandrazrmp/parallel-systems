#!/usr/bin/env python3
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os

sns.set(style="whitegrid")
csv_file = "results/raw_results.csv"
df = pd.read_csv(csv_file)

# Average over repeats
df_avg = df.groupby(['N','sparsity','iterations','threads']).mean().reset_index()

outdir = "results/plots"
os.makedirs(outdir, exist_ok=True)

# Plot CSR vs Dense times, plus breakdown of CSR
for N in df_avg['N'].unique():
    for sparsity in df_avg['sparsity'].unique():
        subset = df_avg[(df_avg['N']==N) & (df_avg['sparsity']==sparsity)]
        if subset.empty:
            continue

        plt.figure(figsize=(8,6))
        plt.plot(subset['threads'], subset['CSR_total_time'], marker='o', label='CSR Total')
        plt.plot(subset['threads'], subset['Dense_time'], marker='o', label='Dense')
        plt.fill_between(subset['threads'], 0, subset['CSR_formation_time'], color='orange', alpha=0.3, label='CSR Formation')
        plt.fill_between(subset['threads'], subset['CSR_formation_time'], subset['CSR_total_time'], color='blue', alpha=0.3, label='CSR Multiplication')
        plt.title(f'Matrix Size={N}, Sparsity={sparsity}%')
        plt.xlabel('Threads')
        plt.ylabel('Time (seconds)')
        plt.legend()
        plt.xticks(subset['threads'])
        plt.grid(True)
        plt.tight_layout()
        plt.savefig(os.path.join(outdir, f'perf_N{N}_S{sparsity}.png'))
        plt.close()

print(f"✅ Plots saved in {outdir}")
