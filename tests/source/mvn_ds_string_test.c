#include "mvn_ds_string_test.h"

#include "mvn_ds/mvn_ds.h"
#include "mvn_ds_test_utils.h"

#include <stdio.h>
#include <string.h>

/**
 * \brief           Run all hmap tests
 * \param[out]      passed_tests: Pointer to passed tests counter
 * \param[out]      failed_tests: Pointer to failed tests counter
 * \param[out]      total_tests: Pointer to total tests counter
 */
int run_hmap_tests(int *passed_tests, int *failed_tests, int *total_tests)
{
    printf("\n===== RUNNING STRING TESTS =====\n");

    int passed_before = *passed_tests;
    int failed_before = *failed_tests;

    //  RUN_TEST(test_basic_string);

    int tests_run = (*passed_tests - passed_before) + (*failed_tests - failed_before);
    (*total_tests) += tests_run;

    return *passed_tests - passed_before;

    printf("\n");
}

int main(void)
{
    int passed = 0;
    int failed = 0;
    int total  = 0;

    run_hmap_tests(&passed, &failed, &total);

    printf("\n===== STRING TEST SUMMARY =====\n");
    print_test_summary(total, passed, failed);

    return (failed > 0) ? 1 : 0;
}
