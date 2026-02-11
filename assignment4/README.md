# Assignment 4 — SIMD-Instruction Programming & CUDA

Assignment 4.

## Description
Implementation of exercise 1 of Assignment 4 covering SIMD-Instruction Programming and measured performance.

## Contents
- `src/` — source code
- `Makefile` — build rules

## Build
From project root:
- make

## Run
- ./bin/poly_mult [polynomial_degree_n]

Run experiments:

```bash
./scripts/run_experiments.sh
```

Output: `results/results_4.1.csv`

Generate plots:

```bash
python3 src/plot_results.py results/results_4.1.csv
```