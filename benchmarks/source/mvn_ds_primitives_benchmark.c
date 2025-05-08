#include "mvn_ds/mvn_ds.h"
#include "mvn_ds_benchmark_utils.h"

#include <stdio.h>

int main()
{
    const size_t num_iterations = 1000000;

    // Benchmark primitive creation
    clock_t start = benchmark_start();
    for (size_t i = 0; i < num_iterations; ++i) {
        mvn_val_t val = mvn_val_i32((int)i);
        mvn_val_free(&val);
    }
    benchmark_end(start, "Primitive Creation and Freeing (I32, 1M iterations)");

    // Benchmark primitive equality
    mvn_val_t val1 = mvn_val_i32(42);
    mvn_val_t val2 = mvn_val_i32(42);
    start          = benchmark_start();
    for (size_t i = 0; i < num_iterations; ++i) {
        mvn_val_equal(&val1, &val2);
    }
    benchmark_end(start, "Primitive Equality Check (I32, 1M iterations)");
    mvn_val_free(&val1);
    mvn_val_free(&val2);

    // Benchmark primitive access
    mvn_val_t val_access = mvn_val_i32(12345);
    start                = benchmark_start();
    for (size_t i = 0; i < num_iterations; ++i) {
        volatile int value = val_access.i32; // Use volatile to prevent compiler optimization
        (void)value;
    }
    benchmark_end(start, "Primitive Access (I32, 1M iterations)");
    mvn_val_free(&val_access);

    return 0;
}
