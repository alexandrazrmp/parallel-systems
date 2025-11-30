import sys
import os
import pandas as pd
import matplotlib.pyplot as plt

OUTPUT_DIR = "results"
os.makedirs(OUTPUT_DIR, exist_ok=True)

def plot_time_vs_transactions(df):
    lock_names = {1: "Coarse Mutex", 2: "Fine Mutex", 3: "Coarse RWLock", 4: "Fine RWLock"}
    elements = sorted(df["elements"].unique())

    for elems in elements:
        subset = df[df["elements"] == elems]

        plt.figure()
        for lock in sorted(df["lock_type"].unique()):
            lock_data = subset[subset["lock_type"] == lock]
            avg = lock_data.groupby("transactions")["time"].mean()

            plt.plot(avg.index, avg.values, marker='o', label=lock_names.get(lock, f"Lock {lock}"))

        plt.xlabel("Transactions per Thread")
        plt.ylabel("Execution Time (s)")
        plt.title(f"Time vs Transactions (Elements={elems})")
        plt.legend()
        plt.grid(True)
        plt.savefig(f"{OUTPUT_DIR}/time_vs_transactions_elems_{elems}.png")
        plt.close()

def plot_time_vs_elements(df):
    lock_names = {1: "Coarse Mutex", 2: "Fine Mutex", 3: "Coarse RWLock", 4: "Fine RWLock"}
    transactions = sorted(df["transactions"].unique())

    for trans in transactions:
        subset = df[df["transactions"] == trans]

        plt.figure()
        for lock in sorted(df["lock_type"].unique()):
            lock_data = subset[subset["lock_type"] == lock]
            avg = lock_data.groupby("elements")["time"].mean()

            plt.plot(avg.index, avg.values, marker='o', label=lock_names.get(lock, f"Lock {lock}"))

        plt.xlabel("Elements (Accounts)")
        plt.ylabel("Execution Time (s)")
        plt.title(f"Time vs Elements (Transactions={trans})")
        plt.legend()
        plt.grid(True)
        plt.savefig(f"{OUTPUT_DIR}/time_vs_elements_trans_{trans}.png")
        plt.close()

def plot_time_vs_query_percentage(df):
    lock_names = {1: "Coarse Mutex", 2: "Fine Mutex", 3: "Coarse RWLock", 4: "Fine RWLock"}

    plt.figure()
    for lock in sorted(df["lock_type"].unique()):
        subset = df[df["lock_type"] == lock]
        avg = subset.groupby("query_percentage")["time"].mean()

        plt.plot(avg.index, avg.values, marker='o', label=lock_names.get(lock, f"Lock {lock}"))

    plt.xlabel("Query Percentage (%)")
    plt.ylabel("Execution Time (s)")
    plt.title("Time vs Query %")
    plt.legend()
    plt.grid(True)
    plt.savefig(f"{OUTPUT_DIR}/time_vs_query_percentage.png")
    plt.close()

def plot_lock_type_comparison(df):
    lock_names = {1: "Coarse Mutex", 2: "Fine Mutex", 3: "Coarse RWLock", 4: "Fine RWLock"}

    avg = df.groupby("lock_type")["time"].mean()

    plt.figure()
    plt.bar([lock_names.get(l, f"Lock {l}") for l in avg.index], avg.values)
    plt.ylabel("Execution Time (s)")
    plt.title("Average Time per Lock Type")
    plt.grid(axis="y")
    plt.xticks(rotation=15)
    plt.tight_layout()
    plt.savefig(f"{OUTPUT_DIR}/lock_type_comparison.png")
    plt.close()

def main():
    if len(sys.argv) != 2:
        print("Usage: python3 shared_var_plot.py <csv_file>")
        sys.exit(1)

    csv_file = sys.argv[1]
    df = pd.read_csv(csv_file)

    print("Generating plots...")

    plot_time_vs_transactions(df)
    plot_time_vs_elements(df)
    plot_time_vs_query_percentage(df)
    plot_lock_type_comparison(df)

    print("Done. PNG files saved in results/.")

if __name__ == "__main__":
    main()
