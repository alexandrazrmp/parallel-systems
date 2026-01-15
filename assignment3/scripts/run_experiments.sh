#!/usr/bin/env bash
set -euo pipefail

# ---------- Paths ----------
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BASEDIR="$(cd "$SCRIPT_DIR/.." && pwd)"

EXEC="$BASEDIR/bin/sparse_matrix_vector_mult"
MACHINES="$BASEDIR/machines"

OUTDIR="$BASEDIR/results"
RAW_CSV="$OUTDIR/raw_results.csv"

mkdir -p "$OUTDIR"

# ---------- Experiment Parameters ----------
MATRIX_SIZES=(1000 5000 10000)
SPARSITY_LIST=(0 25 50 75 99)
ITERATIONS_LIST=(1 5 10 20)
PROCS_LIST=(4 8 16)
REPEATS=4

# ---------- CSV Header ----------
echo "N,sparsity,iterations,processes,run,CSR_construction,CSR_msg_send,CSR_parallel,Total_parallel_CSR,Total_serial_CSR,Parallel_dense,Serial_dense" > "$RAW_CSV"

# ---------- Helper: Extract Time ----------
extract_time() {
    local output="$1"
    local label="$2"
    echo "$output" | grep "$label" | awk -F':' '{print $2}' | awk '{print $1}'
}

# ---------- Run Experiments ----------
for N in "${MATRIX_SIZES[@]}"; do
  for SP in "${SPARSITY_LIST[@]}"; do
    for IT in "${ITERATIONS_LIST[@]}"; do
      for P in "${PROCS_LIST[@]}"; do

        # No rank should have 0 rows
        if (( P > N )); then continue; fi

        for ((run=1; run<=REPEATS; run++)); do
          echo "Running N=$N SP=$SP IT=$IT P=$P run=$run"

          OUT="$(mpiexec -f "$MACHINES" -n "$P" "$EXEC" "$N" "$SP" "$IT" 2>&1)"

          # Verify correctness
          if ! echo "$OUT" | grep -q "All results are correct!"; then
              echo "❌ ERROR in run:"
              echo "$OUT"
              exit 1
          fi

          CSR_CONS=$(extract_time "$OUT" "CSR construction time")
          CSR_MSG=$(extract_time "$OUT" "CSR message send time")
          CSR_PAR=$(extract_time "$OUT" "CSR parallel multiplication time")
          TOTAL_CSR=$(extract_time "$OUT" "Total parallel CSR time")
          TOTAL_CSR_SER=$(extract_time "$OUT" "Total serial CSR time")
          DENSE_PAR=$(extract_time "$OUT" "Total parallel dense time")
          DENSE_SER=$(extract_time "$OUT" "Total serial dense time")

          printf "%d,%d,%d,%d,%d,%s,%s,%s,%s,%s,%s,%s\n" \
            "$N" "$SP" "$IT" "$P" "$run" \
            "$CSR_CONS" "$CSR_MSG" "$CSR_PAR" "$TOTAL_CSR" "$TOTAL_CSR_SER" "$DENSE_PAR" "$DENSE_SER" \
            >> "$RAW_CSV"
        done
      done
    done
  done
done

echo ""
echo "✅ All experiments finished."
echo "📁 Results stored in: $RAW_CSV"
echo ""
echo "To plot results:"
echo "python3 src/plot_results.py"
