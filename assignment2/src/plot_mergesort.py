import matplotlib
matplotlib.use('Agg')

import sys
import os
import pandas as pd
import matplotlib.pyplot as plt

def main():
    # READ CSV FILE
    if len(sys.argv) < 2:
        print("Usage: python3 plots_mergesort.py <results_file.csv>")
        sys.exit(1)

    csv_path = sys.argv[1]
    
    try:
        df = pd.read_csv(csv_path)
    except Exception as e:
        print(f"Error reading CSV: {e}")
        sys.exit(1)

    # Create output folder
    out_dir = "../plots"
    os.makedirs(out_dir, exist_ok=True)

    # PROCESS DATA
    # Prepare data structure for plotting
    plot_data = {}

    # Get unique N values (e.g., 10^7, 10^8)
    unique_Ns = sorted(df['N'].unique())

    for N in unique_Ns:
        subset = df[df['N'] == N]
        
        # Calculate Serial Baseline (Algorithm 0)
        serial_subset = subset[subset['algorithm'] == 0]
        if serial_subset.empty:
            print(f"Warning: No serial data for N={N}. Skipping.")
            continue
            
        avg_serial = serial_subset['execution_time'].mean()

        # Calculate Parallel Data (Algorithm 1)
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

    # PLOTTING

    # Plot 1: Execution Time vs Threads
    plt.figure(figsize=(10, 6))
    colors = ['blue', 'red', 'green', 'orange']

    for i, (N, d) in enumerate(plot_data.items()):
        color = colors[i % len(colors)]
        plt.errorbar(d["threads"], d["times"], yerr=d["std"], 
                     marker='o', label=f'N={N}', color=color, capsize=5)

    plt.xlabel("Threads")
    plt.ylabel("Time (s)")
    plt.title("Mergesort Execution Time vs Threads")
    plt.yscale('log') # Log scale allows seeing both 10^7 and 10^8 clearly
    plt.legend()
    plt.grid(True, which="both", alpha=0.5)
    plt.savefig(os.path.join(out_dir, "mergesort_time.png"), dpi=300)
    plt.close()
    print(f"Saved {out_dir}/mergesort_time.png")

    # Plot 2: Speedup vs Threads
    plt.figure(figsize=(10, 6))

    # Ideal line
    all_threads = [th for d in plot_data.values() for th in d["threads"]]
    if all_threads:
        max_th = max(all_threads)
        plt.plot([1, max_th], [1, max_th], 'k--', alpha=0.5, label="Ideal")

    for i, (N, d) in enumerate(plot_data.items()):
        color = colors[i % len(colors)]
        plt.plot(d["threads"], d["speedup"], marker='o', label=f'N={N}', color=color)

    plt.xlabel("Threads")
    plt.ylabel("Speedup")
    plt.title("Mergesort Speedup vs Threads")
    plt.legend()
    plt.grid(True)
    plt.savefig(os.path.join(out_dir, "mergesort_speedup.png"), dpi=300)
    plt.close()
    print(f"Saved {out_dir}/mergesort_speedup.png")

if __name__ == "__main__":
    main()