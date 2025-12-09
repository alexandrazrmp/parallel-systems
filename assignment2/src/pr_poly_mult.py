#!/usr/bin/env python3
import sys
import csv
import os
import matplotlib.pyplot as plt
from collections import defaultdict

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 src/plot_results.py <results.csv>")
        sys.exit(1)

    csv_path = sys.argv[1]
    out_dir = os.path.dirname(csv_path)
    
    # Data structure: data[degree][threads] = [list of times]
    serial_data = defaultdict(lambda: defaultdict(list))
    parallel_data = defaultdict(lambda: defaultdict(list))

    # 1. Read CSV
    try:
        with open(csv_path, 'r') as f:
            reader = csv.DictReader(f)
            for row in reader:
                n = int(row['degree'])
                th = int(row['threads'])
                s_time = float(row['serial_time'])
                p_time = float(row['parallel_time'])
                
                serial_data[n][th].append(s_time)
                parallel_data[n][th].append(p_time)
    except Exception as e:
        print(f"Error reading CSV: {e}")
        sys.exit(1)

    # 2. Calculate Averages and Speedup
    plot_data = {}

    for n in serial_data.keys():
        threads_list = sorted(serial_data[n].keys())
        avg_p_times = []
        avg_speedups = []

        for th in threads_list:
            # Calculate averages across runs
            avg_serial = sum(serial_data[n][th]) / len(serial_data[n][th])
            avg_parallel = sum(parallel_data[n][th]) / len(parallel_data[n][th])
            
            # Calculate Speedup
            speedup = avg_serial / avg_parallel if avg_parallel > 0 else 0

            avg_p_times.append(avg_parallel)
            avg_speedups.append(speedup)

        plot_data[n] = {
            'threads': threads_list,
            'times': avg_p_times,
            'speedup': avg_speedups
        }

    # 3. Generate Plots
    
    # Plot 1: Execution Time vs Threads
    plt.figure(figsize=(10, 6))
    for n, data in sorted(plot_data.items()):
        plt.plot(data['threads'], data['times'], marker='o', label=f'N={n}')
    
    plt.xlabel('Threads')
    plt.ylabel('Time (seconds)')
    plt.title('Execution Time vs Threads')
    plt.legend()
    plt.grid(True)
    plt.savefig(os.path.join(out_dir, 'plot_time.png'))
    plt.close()

    # Plot 2: Speedup vs Threads
    plt.figure(figsize=(10, 6))
    for n, data in sorted(plot_data.items()):
        plt.plot(data['threads'], data['speedup'], marker='o', label=f'N={n}')
    
    # Add ideal speedup line (reference)
    max_th = max(max(d['threads']) for d in plot_data.values())
    plt.plot([1, max_th], [1, max_th], 'k--', alpha=0.5, label='Ideal')

    plt.xlabel('Threads')
    plt.ylabel('Speedup')
    plt.title('Speedup vs Threads')
    plt.legend()
    plt.grid(True)
    plt.savefig(os.path.join(out_dir, 'plot_speedup.png'))
    plt.close()

    print(f"Plots saved in: {out_dir}")

if __name__ == "__main__":
    main()