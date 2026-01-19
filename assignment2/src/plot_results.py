import matplotlib
matplotlib.use('Agg') # Fixes X11 errors on servers

import pandas as pd
import matplotlib.pyplot as plt
import os
import sys

# --- Configuration ---
# Update this path if your Pthreads file is somewhere else
pth_path = "../../assignment1/results/results_1.1.csv"
output_dir = "../plots"

# --- 1. Load Data ---
if len(sys.argv) < 2:
    print("Usage: python3 plots_mergesort.py <mergesort_results.csv>")
    sys.exit(1)

mergesort_path = sys.argv[1]

if not os.path.exists(mergesort_path):
    print(f"Error: Mergesort file not found: {mergesort_path}")
    sys.exit(1)

try:
    # Load Mergesort Data
    df_merge = pd.read_csv(mergesort_path)
    # Clean column names
    df_merge.columns = df_merge.columns.str.strip()

    # Load Pthreads Data (for comparison)
    df_pth = None
    if os.path.exists(pth_path):
        df_pth = pd.read_csv(pth_path)
        df_pth.columns = df_pth.columns.str.strip()
    else:
        print(f"Warning: Pthreads file not found at {pth_path}. Comparison plot will be skipped.")

except Exception as e:
    print(f"Error reading CSVs: {e}")
    sys.exit(1)

# --- 2. Process Mergesort Data ---
plot_data = {}
unique_Ns = sorted(df_merge['N'].unique())

for N in unique_Ns:
    subset = df_merge[df_merge['N'] == N]
    
    # Serial Baseline (Algorithm 0)
    serial_subset = subset[subset['algorithm'] == 0]
    if serial_subset.empty: continue
    avg_serial = serial_subset['execution_time'].mean()

    # Parallel Data (Algorithm 1)
    par_subset = subset[subset['algorithm'] == 1]
    
    threads = sorted(par_subset['threads'].unique())
    times = []
    std_devs = []
    speedups = []

    for th in threads:
        runs = par_subset[par_subset['threads'] == th]['execution_time']
        avg_t = runs.mean()
        std_t = runs.std() if len(runs) > 1 else 0.0
        
        times.append(avg_t)
        std_devs.append(std_t)
        speedups.append(avg_serial / avg_t)

    plot_data[N] = {
        "threads": threads,
        "times": times,
        "std": std_devs,
        "speedup": speedups
    }

# --- 3. Process Pthreads Data (Reference) ---
pth_speedup_data = None
if df_pth is not None:
    # Use the largest dataset for comparison (N=200,000)
    target_deg = 200000
    col_name = 'degree' if 'degree' in df_pth.columns else 'N'
    
    sub_pth = df_pth[df_pth[col_name] == target_deg]
    
    if not sub_pth.empty:
        # Group by threads
        pth_avg = sub_pth.groupby('threads').mean(numeric_only=True).reset_index()
        
        # Calculate speedup (Average Serial / Average Parallel)
        avg_serial_pth = pth_avg['serial_time'].mean()
        pth_avg['speedup'] = avg_serial_pth / pth_avg['parallel_time']
        
        pth_speedup_data = {
            "threads": pth_avg['threads'],
            "speedup": pth_avg['speedup']
        }

# --- 4. Plotting ---
if not os.path.exists(output_dir):
    os.makedirs(output_dir)

print(f"Generating plots in '{output_dir}'...")

colors = ['blue', 'red', 'green']

# Plot 1: Mergesort Execution Time
plt.figure(figsize=(8, 5))
for i, (N, d) in enumerate(plot_data.items()):
    plt.errorbar(d["threads"], d["times"], yerr=d["std"], 
                 marker='o', label=f'Mergesort N={N}', color=colors[i % len(colors)], capsize=5)

plt.xlabel("Threads")
plt.ylabel("Time (s)")
plt.title("Mergesort Execution Time")
plt.yscale('log') # Log scale to see 10^7 and 10^8 clearly
plt.grid(True, which="both", alpha=0.5)
plt.legend()
plt.tight_layout()
plt.savefig(f'{output_dir}/mergesort_time.png')

# Plot 2: Mergesort Speedup
plt.figure(figsize=(8, 5))
# Ideal line
max_th = 16
plt.plot([1, max_th], [1, max_th], 'k--', alpha=0.5, label="Ideal")

for i, (N, d) in enumerate(plot_data.items()):
    plt.plot(d["threads"], d["speedup"], marker='o', label=f'Mergesort N={N}', color=colors[i % len(colors)])

plt.xlabel("Threads")
plt.ylabel("Speedup")
plt.title("Mergesort Speedup")
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.savefig(f'{output_dir}/mergesort_speedup.png')

# Plot 3: Comparison (Mergesort vs Pthreads Scalability)
if pth_speedup_data:
    plt.figure(figsize=(8, 5))
    
    # Plot Mergesort
    for i, (N, d) in enumerate(plot_data.items()):
        plt.plot(d["threads"], d["speedup"], marker='o', label=f'Mergesort (OpenMP) N={N}', color=colors[i % len(colors)])
    
    # Plot Pthreads
    plt.plot(pth_speedup_data["threads"], pth_speedup_data["speedup"], 
             marker='^', linestyle='--', color='purple', label='PolyMult (Pthreads) N=200k')

    plt.xlabel("Threads")
    plt.ylabel("Speedup")
    plt.title("Scalability Comparison: OpenMP Tasks vs Pthreads")
    plt.grid(True)
    plt.legend()
    plt.tight_layout()
    plt.savefig(f'{output_dir}/comparison_speedup.png')

print("Done!")