#!/bin/bash

set -e

# Define time limits for each benchmark (in seconds)
declare -A BENCHMARK_LIMITS=(
    ["mvn_ds_arr_benchmark"]=5
    ["mvn_ds_hmap_benchmark"]=5
    ["mvn_ds_string_benchmark"]=5
    ["mvn_ds_primitives_benchmark"]=5
)

# Directory containing the benchmark executables
BENCHMARK_DIR="./build/benchmarks"

# Run each benchmark and check its execution time
for BENCHMARK in "${!BENCHMARK_LIMITS[@]}"; do
    BENCHMARK_PATH="${BENCHMARK_DIR}/${BENCHMARK}"
    TIME_LIMIT=${BENCHMARK_LIMITS[$BENCHMARK]}

    if [[ ! -x "$BENCHMARK_PATH" ]]; then
        echo "Error: Benchmark executable $BENCHMARK_PATH not found or not executable."
        exit 1
    fi

    echo "Running $BENCHMARK..."
    START_TIME=$(date +%s)
    $BENCHMARK_PATH
    END_TIME=$(date +%s)

    ELAPSED_TIME=$((END_TIME - START_TIME))
    echo "$BENCHMARK completed in $ELAPSED_TIME seconds."

    if (( ELAPSED_TIME > TIME_LIMIT )); then
        echo "Error: $BENCHMARK exceeded the time limit of $TIME_LIMIT seconds."
        exit 1
    fi
done

echo "All benchmarks completed within their time limits."
