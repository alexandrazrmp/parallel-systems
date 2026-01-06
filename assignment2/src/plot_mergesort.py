#!/usr/bin/env python3
import sys
import csv
import os
import matplotlib.pyplot as plt
from collections import defaultdict

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 plot_mergesort_results.py <raw_results.csv>")
        sys.exit(1)

    csv_path = sys.argv[1]
    out_dir = os.path.dirname(csv_path)
    if out_dir == "":
        out_dir = "results"

    os.makedirs(out_dir, exist_ok=True)

    # serial_data[N][threads] = [times]
    # parallel_data[N][threads] = [times]
    serial_data = defaultdict(lambda: defaultdict(list))
    parallel_data = defaultdict(lambda: defaultdict(list))

    # 1. Read CSV
    with open(csv_path, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            N = int(row['N'])
            algo = int(row['algorithm'])   # 0 = serial, 1 = parallel
            threads = int(row['threads'])
            time = float(row['execution_time'])

            if algo == 0:
                serial_data[N][threads].append(time)
            else:
                parallel_data[N][threads].append(time)

    # 2. Compute averages, speedup, efficiency
    plot_data = {}

    for N in sorted(parallel_data.keys()):
        threads_list = sorted(parallel_data[N].keys())

        avg_parallel_times = []
        speedups = []
        efficiencies = []

        avg_serial = sum(serial_data[N][1]) / len(serial_data[N][1])

        for th in threads_list:
            avg_parallel = sum(parallel_data[N][th]) / len(parallel_data[N][th])
            speedup = avg_serial / avg_parallel if avg_parallel > 0 else 0
            efficiency = speedup / th if th > 0 else 0

            avg_parallel_times.append(avg_parallel)
            speedups.append(speedup)
            efficiencies.append(efficiency)

        plot_data[N] = {
            "threads": threads_list,
            "times": avg_parallel_times,
            "speedup": speedups,
            "efficiency": efficiencies,
            "serial_time": avg_serial
        }

    # -------------------------------
    # Plot 1: Execution Time vs Threads
    # -------------------------------
    plt.figure(figsize=(10, 6))
    for N, data in plot_data.items():
        plt.plot(data["threads"], data["times"], marker='o', label=f'N={N}')
    plt.xlabel("Threads")
    plt.ylabel("Execution Time (seconds)")
    plt.title("Mergesort Execution Time vs Threads")
    plt.legend()
    plt.grid(True)
    plt.savefig(os.path.join(out_dir, "plot_time.png"), dpi=300)
    plt.close()

    # -------------------------------
    # Plot 2: Speedup vs Threads
    # -------------------------------
    plt.figure(figsize=(10, 6))
    for N, data in plot_data.items():
        plt.plot(data["threads"], data["speedup"], marker='o', label=f'N={N}')

    max_threads = max(max(d["threads"]) for d in plot_data.values())
    plt.plot([1, max_threads], [1, max_threads], 'k--', alpha=0.5, label="Ideal")

    plt.xlabel("Threads")
    plt.ylabel("Speedup")
    plt.title("Mergesort Speedup vs Threads")
    plt.legend()
    plt.grid(True)
    plt.savefig(os.path.join(out_dir, "plot_speedup.png"), dpi=300)
    plt.close()

    # -------------------------------
    # Plot 3: Parallel Efficiency vs Threads
    # -------------------------------
    plt.figure(figsize=(10, 6))
    for N, data in plot_data.items():
        plt.plot(data["threads"], data["efficiency"], marker='o', label=f'N={N}')

    plt.xlabel("Threads")
    plt.ylabel("Efficiency")
    plt.title("Mergesort Parallel Efficiency vs Threads")
    plt.legend()
    plt.grid(True)
    plt.savefig(os.path.join(out_dir, "plot_efficiency.png"), dpi=300)
    plt.close()

    # -------------------------------
    # Plot 4: Execution Time vs N (fixed threads)
    # -------------------------------
    fixed_threads = sorted(
        set(th for data in plot_data.values() for th in data["threads"])
    )

    plt.figure(figsize=(10, 6))
    for th in fixed_threads:
        Ns = []
        times = []
        for N, data in plot_data.items():
            if th in data["threads"]:
                idx = data["threads"].index(th)
                Ns.append(N)
                times.append(data["times"][idx])

        plt.plot(Ns, times, marker='o', label=f'{th} threads')

    plt.xlabel("Array Size (N)")
    plt.ylabel("Execution Time (seconds)")
    plt.title("Execution Time vs Array Size")
    plt.legend()
    plt.grid(True)
    plt.savefig(os.path.join(out_dir, "plot_time_vs_N.png"), dpi=300)
    plt.close()

    # -------------------------------
    # Plot 5: Serial vs Best Parallel (Bar Chart)
    # -------------------------------
    labels = []
    serial_times = []
    best_parallel_times = []

    for N, data in plot_data.items():
        labels.append(f"N={N}")
        serial_times.append(data["serial_time"])
        best_parallel_times.append(min(data["times"]))

    x = range(len(labels))
    width = 0.35

    plt.figure(figsize=(10, 6))
    plt.bar(x, serial_times, width, label="Serial")
    plt.bar([i + width for i in x], best_parallel_times, width, label="Best Parallel")

    plt.xticks([i + width / 2 for i in x], labels)
    plt.ylabel("Execution Time (seconds)")
    plt.title("Serial vs Best Parallel Execution Time")
    plt.legend()
    plt.grid(axis='y')
    plt.savefig(os.path.join(out_dir, "plot_serial_vs_parallel.png"), dpi=300)
    plt.close()

    # -------------------------------
    # Plot 6: Speedup vs N (fixed threads)
    # -------------------------------
    plt.figure(figsize=(10, 6))

    for th in fixed_threads:
        Ns = []
        speedups = []
        for N, data in plot_data.items():
            if th in data["threads"]:
                idx = data["threads"].index(th)
                Ns.append(N)
                speedups.append(data["speedup"][idx])

        plt.plot(Ns, speedups, marker='o', label=f'{th} threads')

    plt.xlabel("Array Size (N)")
    plt.ylabel("Speedup")
    plt.title("Speedup vs Array Size")
    plt.legend()
    plt.grid(True)
    plt.savefig(os.path.join(out_dir, "plot_speedup_vs_N.png"), dpi=300)
    plt.close()

        # -------------------------------
    # Plot 7: Efficiency vs N (fixed threads)
    # -------------------------------
    plt.figure(figsize=(10, 6))

    for th in fixed_threads:
        Ns = []
        efficiencies = []
        for N, data in plot_data.items():
            if th in data["threads"]:
                idx = data["threads"].index(th)
                Ns.append(N)
                efficiencies.append(data["efficiency"][idx])

        plt.plot(Ns, efficiencies, marker='o', label=f'{th} threads')

    plt.xlabel("Array Size (N)")
    plt.ylabel("Efficiency")
    plt.title("Parallel Efficiency vs Array Size")
    plt.legend()
    plt.grid(True)
    plt.savefig(os.path.join(out_dir, "plot_efficiency_vs_N.png"), dpi=300)
    plt.close()


    print(f"All plots saved in: {out_dir}")

if __name__ == "__main__":
    main()
