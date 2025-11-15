# Assignment 1 — Parallel Systems

Assignment 1.

## Description
Implement the exercises for Assignment 1 covering basic parallel programming concepts and measured performance.

## Contents
- `src/` — source code
- `Makefile` — build rules

## Build
From project root:
- make

## Run
Example:
- ./bin/program [options]

## Experiments & plotting
Quick steps to build, run the experiment driver (writes per-run CSVs) and generate plots:

```bash
cd assignment1
make
# run experiments (example)
./scripts/run_experiments.sh -d "10000 30000 100000 100000" -t "1 2 4 8 16" -r 3

# plot from the raw CSV
python3 src/plot_results.py results/raw_results.csv
```

## Notes
- Just a README template.
