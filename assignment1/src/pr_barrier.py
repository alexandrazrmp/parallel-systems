import sys
from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd


def main():
    root_dir = Path(__file__).resolve().parents[1]
    default_results = root_dir / "results" / "raw_results.csv"
    default_outdir = root_dir / "results"

    csv_path = Path(sys.argv[1]).expanduser() if len(sys.argv) > 1 else default_results
    out_dir = Path(sys.argv[2]).expanduser() if len(sys.argv) > 2 else default_outdir

    if not csv_path.exists():
        print(f"Input CSV not found: {csv_path}")
        sys.exit(1)

    out_dir.mkdir(parents=True, exist_ok=True)

    df = pd.read_csv(csv_path)
    iteration_counts = df['Iterations'].unique()

    for n in iteration_counts:
        subset = df[df['Iterations'] == n]
        plt.figure(figsize=(10, 6))

        for impl in subset['Implementation'].unique():
            impl_data = subset[subset['Implementation'] == impl].sort_values('Threads')
            plt.plot(impl_data['Threads'], impl_data['Total_Time_Sec'], marker='o', label=impl)

        plt.title(f'Barrier Performance (Iterations: {n})')
        plt.xlabel('Number of Threads')
        plt.ylabel('Total Time (seconds)')
        plt.grid(True)
        plt.legend()

        filename = out_dir / f'barrier_plot_N_{n}.png'
        plt.savefig(filename)
        plt.close()
        print(f"Saved plot: {filename}")

    print(f"All plots generated in {out_dir}.")


if __name__ == "__main__":
    main()