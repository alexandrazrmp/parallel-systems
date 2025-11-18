#!/usr/bin/env python3
import sys
import csv
from collections import defaultdict
import matplotlib.pyplot as plt

def read_results(csv_file):
    """Read raw_results.csv and group data by degree."""
    data = defaultdict(lambda: defaultdict(list))
    with open(csv_file, "r") as f:
        reader = csv.DictReader(f)
        for row in reader:
            degree = int(row["degree"])
            threads = int(row["threads"])
            init_time = float(row["init_time"])
            serial_time = float(row["serial_time"])
            parallel_time = float(row["parallel_time"])
            verification = row["verification"]

            data[degree][threads].append({
                "init": init_time,
                "serial": serial_time,
                "parallel": parallel_time,
                "ver": verification,
            })
    return data


def avg(values):
    return sum(values) / len(values)


def main():
    if len(sys.argv) != 2:
        print("Usage: python3 plot_results.py raw_results.csv")
        sys.exit(1)

    csv_file = sys.argv[1]
    data = read_results(csv_file)

    for degree, thread_data in data.items():
        threads_sorted = sorted(thread_data.keys())

        avg_serial = []
        avg_parallel = []
        speedup = []

        for th in threads_sorted:
            serial_times = [run["serial"] for run in thread_data[th]]
            parallel_times = [run["parallel"] for run in thread_data[th]]

            s = avg(serial_times)
            p = avg(parallel_times)

            avg_serial.append(s)
            avg_parallel.append(p)
            speedup.append(s / p if p > 0 else 0)

        # ------------------------------
        # Plot 1: Execution time
        # ------------------------------
        plt.figure()
        plt.plot(threads_sorted, avg_serial, marker="o", label="Serial time")
        plt.plot(threads_sorted, avg_parallel, marker="o", label="Parallel time")
        plt.xlabel("Threads")
        plt.ylabel("Time (ms)")
        plt.title(f"Execution Time vs Threads (degree={degree})")
        plt.legend()
        plt.grid(True)
        plt.savefig(f"execution_times_degree_{degree}.png", dpi=150)

        # ------------------------------
        # Plot 2: Speedup
        # ------------------------------
        plt.figure()
        plt.plot(threads_sorted, speedup, marker="o")
        plt.xlabel("Threads")
        plt.ylabel("Speedup (Serial/Parallel)")
        plt.title(f"Speedup vs Threads (degree={degree})")
        plt.grid(True)
        plt.savefig(f"speedup_degree_{degree}.png", dpi=150)

    print("Plots generated:")
    for degree in data.keys():
        print(f"  execution_times_degree_{degree}.png")
        print(f"  speedup_degree_{degree}.png")


if __name__ == "__main__":
    main()
