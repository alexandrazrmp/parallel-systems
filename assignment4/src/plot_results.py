#!/usr/bin/env python3
"""
Plot results for Exercise 4.1:
Comparison of serial vs SIMD polynomial multiplication
for different polynomial degrees n.

Expected CSV format:
degree,run,init_time,serial_time,simd_time,verification

Produces:
  - Execution time vs polynomial degree
  - Speedup vs polynomial degree (serial / SIMD)

Usage:
  python3 src/plot_results.py results/results_4.1.csv
"""

import sys
import csv
import os
from collections import defaultdict
import matplotlib.pyplot as plt


def read_and_average(csv_path):
    """
    Read raw per-run CSV and compute average times per degree.
    """
    acc = defaultdict(lambda: {
        'serial_sum': 0.0,
        'simd_sum': 0.0,
        'count': 0
    })

    with open(csv_path, newline='') as f:
        reader = csv.DictReader(f)
        for row in reader:
            degree = int(row['degree'])
            acc[degree]['serial_sum'] += float(row['serial_time'])
            acc[degree]['simd_sum'] += float(row['simd_time'])
            acc[degree]['count'] += 1

    data = {}
    for degree, v in acc.items():
        cnt = v['count']
        avg_serial = v['serial_sum'] / cnt
        avg_simd = v['simd_sum'] / cnt
        speedup = avg_serial / avg_simd if avg_simd > 0 else 0.0

        data[degree] = {
            'serial': avg_serial,
            'simd': avg_simd,
            'speedup': speedup
        }

    return data


def plot_results(outdir, data):
    degrees = sorted(data.keys())
    serial_times = [data[d]['serial'] for d in degrees]
    simd_times = [data[d]['simd'] for d in degrees]
    speedups = [data[d]['speedup'] for d in degrees]

    # -------------------------------------------------
    # Plot 1: Execution time vs polynomial degree
    # -------------------------------------------------
    plt.figure()
    plt.plot(degrees, serial_times, marker='o', label='Serial')
    plt.plot(degrees, simd_times, marker='o', label='SIMD')
    plt.xlabel('Polynomial degree n')
    plt.ylabel('Execution time (seconds)')
    plt.title('Serial vs SIMD Polynomial Multiplication')
    plt.legend()
    plt.grid(True)
    plt.savefig(os.path.join(outdir, 'execution_time_vs_degree.png'))
    plt.close()

    # -------------------------------------------------
    # Plot 2: Speedup vs polynomial degree
    # -------------------------------------------------
    plt.figure()
    plt.plot(degrees, speedups, marker='o')
    plt.axhline(1.0, linestyle='--', label='No speedup')
    plt.xlabel('Polynomial degree n')
    plt.ylabel('Speedup (serial / SIMD)')
    plt.title('SIMD Speedup vs Polynomial Degree')
    plt.legend()
    plt.grid(True)
    plt.savefig(os.path.join(outdir, 'speedup_vs_degree.png'))
    plt.close()

    print("Plots saved in:", outdir)


def main():
    if len(sys.argv) != 2:
        print("Usage: plot_results.py <results_4.1.csv>")
        sys.exit(1)

    csv_path = sys.argv[1]
    if not os.path.exists(csv_path):
        print("Error: CSV file not found:", csv_path)
        sys.exit(2)

    outdir = os.path.dirname(csv_path) or '.'
    data = read_and_average(csv_path)
    plot_results(outdir, data)


if __name__ == '__main__':
    main()
