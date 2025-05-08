#ifndef MVN_DS_BENCHMARK_UTILS_H
#define MVN_DS_BENCHMARK_UTILS_H

#include <stdio.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Starts a timer and returns the start time.
 * @return The start time as a `clock_t` value.
 */
static inline clock_t benchmark_start(void)
{
    return clock();
}

/**
 * @brief Ends a timer and prints the elapsed time in milliseconds.
 * @param start The start time returned by `benchmark_start`.
 * @param label A label to identify the benchmark.
 */
static inline void benchmark_end(clock_t start, const char *label)
{
    clock_t end        = clock();
    double  elapsed_ms = ((double)(end - start) / CLOCKS_PER_SEC) * 1000.0;
    printf("%s: %.3f ms\n", label, elapsed_ms);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MVN_DS_BENCHMARK_UTILS_H */
