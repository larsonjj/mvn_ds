#ifndef MVN_DS_HMAP_TEST_H
#define MVN_DS_HMAP_TEST_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \brief           Run all hmap tests
 * \param[out]      passed: Pointer to integer to increment for passed tests
 * \param[out]      failed: Pointer to integer to increment for failed tests
 * \param[out]      total: Pointer to integer to increment for total tests run
 */
int run_hmap_tests(int *passed, int *failed, int *total);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MVN_DS_HMAP_TEST_H */
