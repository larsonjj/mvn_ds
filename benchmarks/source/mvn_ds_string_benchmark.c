#include "mvn_ds/mvn_ds.h"
#include "mvn_ds_benchmark_utils.h"

#include <stdio.h>

int main()
{
    const size_t num_iterations = 100000;

    // Benchmark string creation
    clock_t start = benchmark_start();
    for (size_t i = 0; i < num_iterations; ++i) {
        mvn_str_t *str = mvn_str_new("benchmark");
        mvn_str_free(str);
    }
    benchmark_end(start, "String Creation and Destruction (100K iterations)");

    // Benchmark string appending
    mvn_str_t *str = mvn_str_new("start");
    if (!str) {
        fprintf(stderr, "Failed to create string for appending benchmark.\n");
        return 1;
    }

    start = benchmark_start();
    for (size_t i = 0; i < num_iterations; ++i) {
        mvn_str_append_cstr(str, "_append");
    }
    benchmark_end(start, "String Appending (100K iterations)");

    mvn_str_free(str);

    return 0;
}
