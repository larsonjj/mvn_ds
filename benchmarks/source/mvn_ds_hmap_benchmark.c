#include "mvn_ds/mvn_ds.h"
#include "mvn_ds_benchmark_utils.h"

#include <stdio.h>

int main()
{
    const size_t num_elements = 100000;

    // Benchmark hash map creation
    clock_t     start = benchmark_start();
    mvn_hmap_t *hmap  = mvn_hmap_new();
    benchmark_end(start, "Hash Map Creation");

    // Benchmark hash map insertion
    start = benchmark_start();
    for (size_t i = 0; i < num_elements; ++i) {
        char key[32];
        snprintf(key, sizeof(key), "key%zu", i);
        mvn_hmap_set_cstr(hmap, key, mvn_val_i32((int)i));
    }
    benchmark_end(start, "Hash Map Insertion (100K elements)");

    // Benchmark hash map lookup
    start = benchmark_start();
    for (size_t i = 0; i < num_elements; ++i) {
        char key[32];
        snprintf(key, sizeof(key), "key%zu", i);
        mvn_val_t *val = mvn_hmap_get_cstr(hmap, key);
        if (val == NULL || val->type != MVN_VAL_I32 || val->i32 != (int)i) {
            fprintf(stderr, "Hash map lookup error for key %s\n", key);
        }
    }
    benchmark_end(start, "Hash Map Lookup (100K elements)");

    // Free the hash map
    mvn_hmap_free(hmap);

    return 0;
}
