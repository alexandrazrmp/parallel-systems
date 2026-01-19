import matplotlib
matplotlib.use('Agg') # Use non-interactive backend

import pandas as pd
import matplotlib.pyplot as plt
import os

# --- Configuration ---
# File paths
mpi_path = "../results/multinode_results_final.csv"
omp_path = "../../assignment2/results/results_2.1_omp.csv"
pth_path = "../../assignment1/results/results_1.1.csv"

# Target dataset size
N_TARGET = 200000

# --- Validation ---
def check_path(path):
    if not os.path.exists(path):
        print(f"Error: File not found at {path}")
        return False
    return True

if not (check_path(mpi_path) and check_path(omp_path) and check_path(pth_path)):
    exit(1)

# --- Data Loading ---
try:
    df_mpi = pd.read_csv(mpi_path)
    df_omp = pd.read_csv(omp_path)
    df_pth = pd.read_csv(pth_path)
except Exception as e:
    print(f"Error reading CSVs: {e}")
    exit(1)

# --- Processing ---
# Filter data for N=200,000
df_mpi = df_mpi[df_mpi['N'] == N_TARGET]
df_omp = df_omp[df_omp['degree'] == N_TARGET]
df_pth = df_pth[df_pth['degree'] == N_TARGET]

if df_mpi.empty:
    print(f"Warning: No MPI data found for N={N_TARGET}")

# Calculate averages
mpi_avg = df_mpi.groupby('Nodes').mean(numeric_only=True).reset_index()
omp_avg = df_omp.groupby('threads').mean(numeric_only=True).reset_index()
pth_avg = df_pth.groupby('threads').mean(numeric_only=True).reset_index()

# --- Plotting ---
if not os.path.exists("../plots"):
    os.makedirs("../plots")

print(f"Generating plots for N={N_TARGET}...")

# 1. MPI Total Execution Time
plt.figure(figsize=(8, 5))
plt.plot(mpi_avg['Nodes'], mpi_avg['Total_Time'], marker='o', color='blue')
plt.title(f'MPI Total Execution Time (N={N_TARGET})')
plt.xlabel('Number of Processes')
plt.ylabel('Time (s)')
plt.grid(True)
plt.tight_layout()
plt.savefig('../plots/mpi_total_time.png')
print("Saved mpi_total_time.png")

# 2. MPI Breakdown (Stacked Bar)
plt.figure(figsize=(8, 5))
nodes = mpi_avg['Nodes'].astype(str)
send = mpi_avg['Send_Time'].fillna(0)
comp = mpi_avg['Comp_Time'].fillna(0)
recv = mpi_avg['Recv_Time'].fillna(0)

plt.bar(nodes, send, label='Send', color='#d62728')
plt.bar(nodes, comp, bottom=send, label='Compute', color='#2ca02c')
plt.bar(nodes, recv, bottom=send+comp, label='Receive', color='#ff7f0e')

plt.title(f'MPI Time Breakdown (N={N_TARGET})')
plt.xlabel('MPI Processes')
plt.ylabel('Time (s)')
plt.legend()
plt.tight_layout()
plt.savefig('../plots/mpi_breakdown.png')
print("Saved mpi_breakdown.png")

# 3. Comparison (MPI vs OpenMP vs Pthreads)
plt.figure(figsize=(8, 5))

# Limit MPI to 8 nodes to match threads scale
mpi_subset = mpi_avg[mpi_avg['Nodes'] <= 8]

plt.plot(mpi_subset['Nodes'], mpi_subset['Total_Time'], 
         marker='o', label='MPI (Nodes <= 8)', color='blue')
plt.plot(omp_avg['threads'], omp_avg['parallel_time'], 
         marker='s', label='OpenMP', color='green', linestyle='--')
plt.plot(pth_avg['threads'], pth_avg['parallel_time'], 
         marker='^', label='Pthreads', color='red', linestyle=':')

plt.title(f'Performance Comparison (N={N_TARGET})')
plt.xlabel('Processes / Threads')
plt.ylabel('Time (s)')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('../plots/comparison_all.png')
print("Saved comparison_all.png")

print("Done!")