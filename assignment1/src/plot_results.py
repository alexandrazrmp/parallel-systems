#!/usr/bin/env python3
"""
Plotting script that accepts either:
 - a summary CSV with columns: degree,threads,avg_serial,avg_parallel,speedup[,passes]
 - a raw per-run CSV with columns: degree,threads,run,init_time,serial_time,parallel_time,verification

If given a raw CSV the script will aggregate averages per (degree,threads) and compute speedup.
Usage: python3 src/plot_results.py results/raw_results.csv
"""

import sys
import csv
import os
from collections import defaultdict
import matplotlib.pyplot as plt


def read_and_aggregate(path):
    outdir = os.path.dirname(path) or '.'
    # Read CSV and detect format
    with open(path, newline='') as f:
        rdr = csv.DictReader(f)
        fields = rdr.fieldnames or []
        # If the CSV already contains averages, pass through
        if 'avg_serial' in fields and 'avg_parallel' in fields and 'speedup' in fields:
            data = defaultdict(lambda: {'threads': [], 'avg_serial': [], 'avg_parallel': [], 'speedup': [], 'passes': []})
            for row in rdr:
                degree = int(row['degree'])
                th = int(row['threads'])
                avg_serial = float(row['avg_serial'])
                avg_parallel = float(row['avg_parallel'])
                speedup = float(row['speedup'])
                passes = int(row.get('passes', 0))
                data[degree]['threads'].append(th)
                data[degree]['avg_serial'].append(avg_serial)
                data[degree]['avg_parallel'].append(avg_parallel)
                data[degree]['speedup'].append(speedup)
                data[degree]['passes'].append(passes)
            return outdir, data

        # Otherwise assume raw per-run CSV and aggregate
        agg = {}  # (degree,threads) -> accumulators
        for row in rdr:
            try:
                degree = int(row['degree'])
                th = int(row['threads'])
            except Exception:
                continue
            key = (degree, th)
            if key not in agg:
                agg[key] = {'sum_init': 0.0, 'sum_serial': 0.0, 'sum_parallel': 0.0, 'count': 0, 'passes': 0}
            a = agg[key]
            a['sum_init'] += float(row.get('init_time', 0.0) or 0.0)
            a['sum_serial'] += float(row.get('serial_time', 0.0) or 0.0)
            a['sum_parallel'] += float(row.get('parallel_time', 0.0) or 0.0)
            a['count'] += 1
            if row.get('verification', '').strip().upper() == 'PASS':
                a['passes'] += 1

        # build data structure compatible with plotting section
        data = defaultdict(lambda: {'threads': [], 'avg_serial': [], 'avg_parallel': [], 'speedup': [], 'passes': []})
        for (degree, th), a in agg.items():
            cnt = a['count'] if a['count'] > 0 else 1
            avg_serial = a['sum_serial'] / cnt
            avg_parallel = a['sum_parallel'] / cnt
            speedup = (avg_serial / avg_parallel) if avg_parallel > 0 else 0.0
            data[degree]['threads'].append(th)
            data[degree]['avg_serial'].append(avg_serial)
            data[degree]['avg_parallel'].append(avg_parallel)
            data[degree]['speedup'].append(speedup)
            data[degree]['passes'].append(a.get('passes', 0))
        return outdir, data


def plot_from_data(outdir, data):
    # Prepare combined plotting structures
    combined = {}
    # combined[degree] = {'threads': [...], 'avg_parallel': [...], 'speedup': [...], 'serial': ...}
    for degree, grp in data.items():
        zipped = sorted(zip(grp['threads'], grp['avg_serial'], grp['avg_parallel'], grp['speedup']))
        threads, serials, parallels, speedups = zip(*zipped)
        # Store avg_serial per-thread so we can compute the linear (ideal) time using
        # ideal_time = avg_serial / threads (avg_serial is the serial time measured in the same run).
        combined[degree] = {
            'threads': list(threads),
            'avg_serial': list(serials),
            'avg_parallel': list(parallels),
            'speedup': list(speedups),
        }

    # Per-degree plots removed per request. We only produce combined plots below.

    # --- Combined plots across degrees ---
    # Time vs threads (parallel times for each degree) + ideal time curves (serial/threads)
    plt.figure()
    for degree in sorted(combined.keys()):
        d = combined[degree]
        threads = d['threads']
        plt.plot(threads, d['avg_parallel'], marker='o', label=f'degree={degree}')
    plt.xlabel('Threads')
    plt.ylabel('Parallel time (s)')
    plt.title('Parallel execution time vs threads')
    plt.legend()
    plt.grid(True)
    plt.savefig(os.path.join(outdir, 'time_vs_threads.png'))
    plt.close()

    # Speedup vs threads (for each degree) + ideal linear reference y = threads
    plt.figure()
    for degree in sorted(combined.keys()):
        d = combined[degree]
        plt.plot(d['threads'], d['speedup'], marker='o', label=f'degree={degree}')
    plt.xlabel('Threads')
    plt.ylabel('Speedup')
    plt.title('Speedup vs threads')
    plt.legend()
    plt.grid(True)
    plt.savefig(os.path.join(outdir, 'speedup_vs_threads.png'))
    plt.close()

    # Remove any old per-degree PNGs to keep only the combined plots if they exist
    try:
        import glob
        for p in glob.glob(os.path.join(outdir, 'time_vs_threads_degree_*.png')):
            os.remove(p)
        for p in glob.glob(os.path.join(outdir, 'speedup_vs_threads_degree_*.png')):
            os.remove(p)
    except Exception:
        pass

    print('Plots written to', outdir)


def main():
    if len(sys.argv) < 2:
        print('Usage: plot_results.py <summary_or_raw.csv>')
        sys.exit(1)
    path = sys.argv[1]
    if not os.path.exists(path):
        print(f"Error: CSV file not found: {path}")
        print("")
        print("This script expects a per-run raw CSV (canonical): results/raw_results.csv")
        print("To produce it, run the experiment driver from the assignment1 directory:")
        print("  make               # build the binary if needed")
        print("  ./scripts/run_experiments.sh -d \"10000 30000 100000\" -t \"1 2 4 8 16\" -r 3")
        print("")
        print("Alternatively, provide a path to an existing summary CSV that contains columns:")
        print("  degree,threads,avg_serial,avg_parallel,speedup")
        sys.exit(2)
    outdir, data = read_and_aggregate(path)
    plot_from_data(outdir, data)


if __name__ == '__main__':
    main()