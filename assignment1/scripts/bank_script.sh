#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BASEDIR="$(cd "$SCRIPT_DIR/.." && pwd)"

BIN="$BASEDIR/bin/bank"
OUTDIR="$BASEDIR/results"
RAW_CSV="$OUTDIR/raw_results.csv"

ELEMENTS_LIST=(1000 10000 50000 100000)
TRANSACTIONS_LIST=(10000 50000 100000 500000)
QUERY_PERCENTAGES=(0 10 30 50 70 90 100)
LOCK_TYPES=(1 2 3 4)
THREADS=4
REPEATS=4

mkdir -p "$OUTDIR"
echo "elements,transactions,query_percentage,lock_type,run,time" > "$RAW_CSV"

for elems in "${ELEMENTS_LIST[@]}"; do
  echo ""
  echo "=== Testing n_elements: $elems ==="

  for trans in "${TRANSACTIONS_LIST[@]}"; do
    echo "  Transactions per thread: $trans"

    for q in "${QUERY_PERCENTAGES[@]}"; do
      echo "    Query % = $q"

      for lock in "${LOCK_TYPES[@]}"; do
        echo "      Lock type: $lock (repeats=$REPEATS)"

        for ((i=1; i<=REPEATS; i++)); do

          # Run program and capture output + exit code
          if ! out="$("$BIN" "$elems" "$trans" "$q" "$lock" "$THREADS" 2>&1)"; then
            echo ""
            echo "❌ ERROR: Program crashed or returned non-zero exit code!"
            echo "Elements=$elems Transactions=$trans Query=$q Lock=$lock Run=$i"
            echo ""
            echo "Output:"
            echo "$out"
            exit 1
          fi

          # Check correctness
          if ! echo "$out" | grep -q "Verification: PASS"; then
            echo ""
            echo "ERROR: Verification failed!"
            echo "Elements=$elems Transactions=$trans Query=$q Lock=$lock Run=$i"
            echo ""
            echo "Program output:"
            echo "$out"
            echo ""
            echo "Aborting to avoid logging invalid results."
            exit 1
          fi

          # Extract execution time safely
          time=$(echo "$out" | grep "Execution time" | awk '{print $3}')
          if [[ -z "$time" ]]; then
            echo "❌ ERROR: Failed to parse execution time!"
            echo "$out"
            exit 1
          fi

          printf "%s,%s,%s,%s,%d,%s\n" \
            "$elems" "$trans" "$q" "$lock" "$i" "$time" >> "$RAW_CSV"

        done
      done
    done
  done
done

echo ""
echo "Done. Raw results saved to: $RAW_CSV"
echo ""
echo "To plot results, run:"
echo "python3 \"$BASEDIR/src/bank_plot.py\" \"$RAW_CSV\""
