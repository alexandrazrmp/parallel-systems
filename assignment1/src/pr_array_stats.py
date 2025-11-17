#!/usr/bin/env python3
"""
Simple plotting script for array_stats experiments.
Reads a raw per-run CSV with columns:
  size,threads,run,init_time,serial_time,parallel_time,verification
Aggregates averages per (size,threads), computes speedup, and writes two plots:
  results/time_vs_threads.png
  results/speedup_vs_threads.png
Usage: python3 src/pr_array_stats.py results/raw_results.csv
"""

import sys
import os
import csv
from collections import defaultdict
import matplotlib.pyplot as plt


def read_and_aggregate(path):
    agg = {}  # (size,threads) -> accumulators
    with open(path, newline='') as f:
        rdr = csv.DictReader(f)
        for row in rdr:
            try:
                size = int(row['size'])
                th = int(row['threads'])
            except Exception:
                continue
            key = (size, th)
            if key not in agg:
                agg[key] = {'sum_init': 0.0, 'sum_serial': 0.0, 'sum_parallel': 0.0, 'count': 0, 'passes': 0}
            a = agg[key]
            a['sum_init'] += float(row.get('init_time', 0.0) or 0.0)
            a['sum_serial'] += float(row.get('serial_time', 0.0) or 0.0)
            a['sum_parallel'] += float(row.get('parallel_time', 0.0) or 0.0)
            a['count'] += 1
            if row.get('verification', '').strip().upper() == 'PASS':
                a['passes'] += 1

    # Build per-size data
    data = defaultdict(lambda: {'threads': [], 'avg_serial': [], 'avg_parallel': [], 'speedup': [], 'passes': []})
    for (size, th), a in agg.items():
        cnt = a['count'] if a['count'] > 0 else 1
        avg_serial = a['sum_serial'] / cnt
        avg_parallel = a['sum_parallel'] / cnt
        speedup = (avg_serial / avg_parallel) if avg_parallel > 0 else 0.0
        data[size]['threads'].append(th)
        data[size]['avg_serial'].append(avg_serial)
        data[size]['avg_parallel'].append(avg_parallel)
        data[size]['speedup'].append(speedup)
        data[size]['passes'].append(a.get('passes', 0))
    return data


def plot(data, outdir):
    os.makedirs(outdir, exist_ok=True)

    # Time vs threads
    plt.figure()
    for size in sorted(data.keys()):
        d = data[size]
        zipped = sorted(zip(d['threads'], d['avg_parallel'], d['avg_serial'], d['speedup']))
        threads, parallels, serials, speedups = zip(*zipped)
        plt.plot(threads, parallels, marker='o', label=f'size={size}')
        # optional: plot ideal (serial / threads) curve using the avg_serial for thread=1 if present
        # compute ideal_time = serial_at_1 / threads
        try:
            # find serial value for threads==1
            if 1 in threads:
                serial_at_1 = serials[list(threads).index(1)]
                ideal = [serial_at_1 / t for t in threads]
                plt.plot(threads, ideal, linestyle='--', color='gray', alpha=0.4)
        except Exception:
            pass

    plt.xlabel('Threads')
    plt.ylabel('Parallel time (s)')
    plt.title('Parallel execution time vs threads')
    plt.legend()
    plt.grid(True)
    time_png = os.path.join(outdir, 'time_vs_threads.png')
    plt.savefig(time_png)
    plt.close()

    # Speedup vs threads
    plt.figure()
    for size in sorted(data.keys()):
        d = data[size]
        zipped = sorted(zip(d['threads'], d['speedup']))
        threads, speedups = zip(*zipped)
        plt.plot(threads, speedups, marker='o', label=f'size={size}')

    plt.xlabel('Threads')
    plt.ylabel('Speedup')
    plt.title('Speedup vs threads')
    plt.legend()
    plt.grid(True)
    speedup_png = os.path.join(outdir, 'speedup_vs_threads.png')
    plt.savefig(speedup_png)
    plt.close()

    print('Plots written to', outdir)


def main():
    if len(sys.argv) != 2:
        print('Usage: pr_array_stats.py results/raw_results.csv')
        sys.exit(1)
    path = sys.argv[1]
    if not os.path.exists(path):
        print('CSV not found:', path)
        sys.exit(2)
    outdir = os.path.dirname(path) or '.'
    data = read_and_aggregate(path)
    plot(data, outdir)


if __name__ == '__main__':
    main()
