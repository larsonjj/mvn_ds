#include "mvn_ds/mvn_ds.h"
#include "mvn_ds_benchmark_utils.h"

#include <stdio.h>

int main()
{
    const size_t num_elements = 1000000;

    // Benchmark array creation
    clock_t    start = benchmark_start();
    mvn_arr_t *array = mvn_arr_new();
    benchmark_end(start, "Array Creation");

    // Benchmark array push
    start = benchmark_start();
    for (size_t i = 0; i < num_elements; ++i) {
        mvn_arr_push(array, mvn_val_i32((int)i));
    }
    benchmark_end(start, "Array Push (1M elements)");

    // Benchmark array access
    start = benchmark_start();
    for (size_t i = 0; i < num_elements; ++i) {
        mvn_val_t *val = mvn_arr_get(array, i);
        if (val == NULL || val->type != MVN_VAL_I32 || val->i32 != (int)i) {
            fprintf(stderr, "Array access error at index %zu\n", i);
        }
    }
    benchmark_end(start, "Array Access (1M elements)");

    // Free the array
    mvn_arr_free(array);

    return 0;
}
