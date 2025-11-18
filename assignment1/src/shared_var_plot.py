#!/usr/bin/env python3
import sys
import os
import pandas as pd
import matplotlib.pyplot as plt

def main():
    if len(sys.argv) != 2:
        print("Usage: python3 shared_var_plot.py <raw_results.csv>")
        sys.exit(1)

    csv_path = sys.argv[1]
    outdir = os.path.dirname(csv_path)
    os.makedirs(outdir, exist_ok=True)

    # Load data
    df = pd.read_csv(csv_path)

    # Ensure numeric types
    df["threads"] = df["threads"].astype(int)
    df["iterations"] = df["iterations"].astype(int)

    # Group by iterations and threads, average the timing results
    grouped = df.groupby(["iterations", "threads"], as_index=False).mean()

    iterations_list = sorted(df["iterations"].unique())

    for it in iterations_list:
        subset = grouped[grouped["iterations"] == it]

        threads = subset["threads"]

        # --- Plot MUTEX ---
        plt.figure()
        plt.plot(threads, subset["mutex_time"], marker='o', label="Mutex")
        plt.xlabel("Threads")
        plt.ylabel("Time (microseconds?)")
        plt.title(f"Mutex Time vs Threads (iterations={it})")
        plt.grid(True)
        plt.legend()
        plt.savefig(f"{outdir}/mutex_iterations_{it}.png")
        plt.close()

        # --- Plot RWLOCK ---
        plt.figure()
        plt.plot(threads, subset["rwlock_time"], marker='o', label="RWLock")
        plt.xlabel("Threads")
        plt.ylabel("Time (microseconds?)")
        plt.title(f"RWLock Time vs Threads (iterations={it})")
        plt.grid(True)
        plt.legend()
        plt.savefig(f"{outdir}/rwlock_iterations_{it}.png")
        plt.close()

        # --- Plot ATOMIC ---
        plt.figure()
        plt.plot(threads, subset["atomic_time"], marker='o', label="Atomic")
        plt.xlabel("Threads")
        plt.ylabel("Time (microseconds?)")
        plt.title(f"Atomic Time vs Threads (iterations={it})")
        plt.grid(True)
        plt.legend()
        plt.savefig(f"{outdir}/atomic_iterations_{it}.png")
        plt.close()

    print(f"Plots saved to: {outdir}")


if __name__ == "__main__":
    main()
