#!/usr/bin/env python3
import sys
import os
import pandas as pd
import matplotlib.pyplot as plt

def main():
    if len(sys.argv) != 2:
        print("Usage: python3 plot_shared_var_comparison.py <raw_results.csv>")
        sys.exit(1)

    csv_path = sys.argv[1]
    outdir = os.path.dirname(csv_path)
    os.makedirs(outdir, exist_ok=True)

    # Load data
    df = pd.read_csv(csv_path)

    # Ensure numeric types
    df["threads"] = df["threads"].astype(int)
    df["iterations"] = df["iterations"].astype(int)

    # Convert microseconds -> seconds if needed
    # If your CSV already contains seconds, remove this division.
    df["mutex_time"]   = df["mutex_time"]   / 1_000_000
    df["rwlock_time"]  = df["rwlock_time"]  / 1_000_000
    df["atomic_time"]  = df["atomic_time"]  / 1_000_000

    # Average results for same (threads, iterations)
    grouped = df.groupby(["iterations", "threads"], as_index=False).mean()

    iterations_list = sorted(grouped["iterations"].unique())

    for it in iterations_list:
        subset = grouped[grouped["iterations"] == it]

        threads = subset["threads"]

        plt.figure()
        plt.plot(threads, subset["mutex_time"], marker='o', label="Mutex")
        plt.plot(threads, subset["rwlock_time"], marker='o', label="RWLock")
        plt.plot(threads, subset["atomic_time"], marker='o', label="Atomic")

        plt.xlabel("Threads")
        plt.ylabel("Parallel Time (seconds)")
        plt.title(f"Performance Comparison (iterations = {it})")
        plt.grid(True)
        plt.legend()

        out_file = f"{outdir}/comparison_iterations_{it}.png"
        plt.savefig(out_file)
        plt.close()
        print(f"Saved: {out_file}")

    print("\nAll comparison plots generated.\n")


if __name__ == "__main__":
    main()
