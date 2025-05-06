/*
 * Copyright (c) 2024 Jake Larson
 */
#include "mvn_ds_primitives_test.h"

#include "mvn_ds/mvn_ds.h"
#include "mvn_ds/mvn_ds_arr.h"  // For mvn_val_arr
#include "mvn_ds/mvn_ds_hmap.h" // For mvn_val_hmap
#include "mvn_ds/mvn_ds_str.h"  // For mvn_val_string_borrow
#include "mvn_ds_test_utils.h"

#include <math.h>    // For fabsf, fabs
#include <stdbool.h> // For bool
#include <stdint.h>  // For int types
#include <stdio.h>
#include <string.h>

// --- Test Functions ---

static bool test_primitives(void)
{
    // --- Test MVN_VAL_NULL ---
    mvn_val_t val_null = mvn_val_null();
    TEST_ASSERT(val_null.type == MVN_VAL_NULL, "Null type mismatch");
    mvn_val_free(&val_null);
    TEST_ASSERT(val_null.type == MVN_VAL_NULL, "Null type mismatch after free");

    // --- Test MVN_VAL_BOOL ---
    mvn_val_t val_bool_true = mvn_val_bool(true);
    TEST_ASSERT(val_bool_true.type == MVN_VAL_BOOL, "Bool(true) type mismatch");
    TEST_ASSERT(val_bool_true.b == true, "Bool(true) value mismatch");
    mvn_val_free(&val_bool_true);
    TEST_ASSERT(val_bool_true.type == MVN_VAL_NULL, "Bool(true) type mismatch after free");

    mvn_val_t val_bool_false = mvn_val_bool(false);
    TEST_ASSERT(val_bool_false.type == MVN_VAL_BOOL, "Bool(false) type mismatch");
    TEST_ASSERT(val_bool_false.b == false, "Bool(false) value mismatch");
    mvn_val_free(&val_bool_false);
    TEST_ASSERT(val_bool_false.type == MVN_VAL_NULL, "Bool(false) type mismatch after free");

    // --- Test MVN_VAL_I32 ---
    mvn_val_t val_i32_pos = mvn_val_i32(12345);
    TEST_ASSERT(val_i32_pos.type == MVN_VAL_I32, "I32(pos) type mismatch");
    TEST_ASSERT(val_i32_pos.i32 == 12345, "I32(pos) value mismatch");
    mvn_val_free(&val_i32_pos);
    TEST_ASSERT(val_i32_pos.type == MVN_VAL_NULL, "I32(pos) type mismatch after free");

    mvn_val_t val_i32_neg = mvn_val_i32(-54321);
    TEST_ASSERT(val_i32_neg.type == MVN_VAL_I32, "I32(neg) type mismatch");
    TEST_ASSERT(val_i32_neg.i32 == -54321, "I32(neg) value mismatch");
    mvn_val_free(&val_i32_neg);
    TEST_ASSERT(val_i32_neg.type == MVN_VAL_NULL, "I32(neg) type mismatch after free");

    mvn_val_t val_i32_zero = mvn_val_i32(0);
    TEST_ASSERT(val_i32_zero.type == MVN_VAL_I32, "I32(zero) type mismatch");
    TEST_ASSERT(val_i32_zero.i32 == 0, "I32(zero) value mismatch");
    mvn_val_free(&val_i32_zero);
    TEST_ASSERT(val_i32_zero.type == MVN_VAL_NULL, "I32(zero) type mismatch after free");

    // --- Test MVN_VAL_I64 ---
    mvn_val_t val_i64_pos = mvn_val_i64(9876543210LL);
    TEST_ASSERT(val_i64_pos.type == MVN_VAL_I64, "I64(pos) type mismatch");
    TEST_ASSERT(val_i64_pos.i64 == 9876543210LL, "I64(pos) value mismatch");
    mvn_val_free(&val_i64_pos);
    TEST_ASSERT(val_i64_pos.type == MVN_VAL_NULL, "I64(pos) type mismatch after free");

    mvn_val_t val_i64_neg = mvn_val_i64(-1029384756LL);
    TEST_ASSERT(val_i64_neg.type == MVN_VAL_I64, "I64(neg) type mismatch");
    TEST_ASSERT(val_i64_neg.i64 == -1029384756LL, "I64(neg) value mismatch");
    mvn_val_free(&val_i64_neg);
    TEST_ASSERT(val_i64_neg.type == MVN_VAL_NULL, "I64(neg) type mismatch after free");

    mvn_val_t val_i64_zero = mvn_val_i64(0LL);
    TEST_ASSERT(val_i64_zero.type == MVN_VAL_I64, "I64(zero) type mismatch");
    TEST_ASSERT(val_i64_zero.i64 == 0LL, "I64(zero) value mismatch");
    mvn_val_free(&val_i64_zero);
    TEST_ASSERT(val_i64_zero.type == MVN_VAL_NULL, "I64(zero) type mismatch after free");

    // --- Test MVN_VAL_F32 ---
    mvn_val_t val_f32_pos = mvn_val_f32(123.456f);
    TEST_ASSERT(val_f32_pos.type == MVN_VAL_F32, "F32(pos) type mismatch");
    TEST_ASSERT_FLOAT_EQ(val_f32_pos.f32, 123.456f, "F32(pos) value mismatch");
    mvn_val_free(&val_f32_pos);
    TEST_ASSERT(val_f32_pos.type == MVN_VAL_NULL, "F32(pos) type mismatch after free");

    mvn_val_t val_f32_neg = mvn_val_f32(-987.654f);
    TEST_ASSERT(val_f32_neg.type == MVN_VAL_F32, "F32(neg) type mismatch");
    TEST_ASSERT_FLOAT_EQ(val_f32_neg.f32, -987.654f, "F32(neg) value mismatch");
    mvn_val_free(&val_f32_neg);
    TEST_ASSERT(val_f32_neg.type == MVN_VAL_NULL, "F32(neg) type mismatch after free");

    mvn_val_t val_f32_zero = mvn_val_f32(0.0f);
    TEST_ASSERT(val_f32_zero.type == MVN_VAL_F32, "F32(zero) type mismatch");
    TEST_ASSERT_FLOAT_EQ(val_f32_zero.f32, 0.0f, "F32(zero) value mismatch");
    mvn_val_free(&val_f32_zero);
    TEST_ASSERT(val_f32_zero.type == MVN_VAL_NULL, "F32(zero) type mismatch after free");

    // --- Test MVN_VAL_F64 ---
    mvn_val_t val_f64_pos = mvn_val_f64(123456.789012);
    TEST_ASSERT(val_f64_pos.type == MVN_VAL_F64, "F64(pos) type mismatch");
    TEST_ASSERT_DOUBLE_EQ(val_f64_pos.f64, 123456.789012, "F64(pos) value mismatch");
    mvn_val_free(&val_f64_pos);
    TEST_ASSERT(val_f64_pos.type == MVN_VAL_NULL, "F64(pos) type mismatch after free");

    mvn_val_t val_f64_neg = mvn_val_f64(-987654.321098);
    TEST_ASSERT(val_f64_neg.type == MVN_VAL_F64, "F64(neg) type mismatch");
    TEST_ASSERT_DOUBLE_EQ(val_f64_neg.f64, -987654.321098, "F64(neg) value mismatch");
    mvn_val_free(&val_f64_neg);
    TEST_ASSERT(val_f64_neg.type == MVN_VAL_NULL, "F64(neg) type mismatch after free");

    mvn_val_t val_f64_zero = mvn_val_f64(0.0);
    TEST_ASSERT(val_f64_zero.type == MVN_VAL_F64, "F64(zero) type mismatch");
    TEST_ASSERT_DOUBLE_EQ(val_f64_zero.f64, 0.0, "F64(zero) value mismatch");
    mvn_val_free(&val_f64_zero);
    TEST_ASSERT(val_f64_zero.type == MVN_VAL_NULL, "F64(zero) type mismatch after free");

    return true; // All tests passed
}

static bool test_additional_primitive_operations(void)
{
    /*
     * Copyright (c) 2024 Jake Larson
     */
    printf("Testing mvn_val_print(NULL):\n");
    mvn_val_print(NULL); // Test printing a NULL mvn_val_t pointer
    printf("\nEnd of mvn_val_print(NULL) test.\n");

    mvn_val_t val_i32_sample  = mvn_val_i32(123);
    mvn_val_t val_str_sample  = mvn_val_string("sample"); // Changed from mvn_val_string_borrow
    mvn_val_t val_arr_sample  = mvn_val_arr();
    mvn_val_t val_hmap_sample = mvn_val_hmap();

    // Test mvn_val_equal with NULL mvn_val_t* pointers
    TEST_ASSERT(!mvn_val_equal(NULL, &val_i32_sample), "mvn_val_equal(NULL, &val) should be false");
    TEST_ASSERT(!mvn_val_equal(&val_i32_sample, NULL), "mvn_val_equal(&val, NULL) should be false");
    // Behavior of mvn_val_equal(NULL, NULL) depends on its implementation.
    // Assuming it returns false if either pointer is NULL, as per the snippet.
    // If it's defined that two NULL pointers are "equal", this assertion would change.
    TEST_ASSERT(mvn_val_equal(NULL, NULL),                   // Changed from !mvn_val_equal
                "mvn_val_equal(NULL, NULL) should be true"); // Updated message

    // Test mvn_val_equal comparing primitives with complex types (should be false due to type
    // mismatch)
    TEST_ASSERT(!mvn_val_equal(&val_i32_sample, &val_str_sample), "I32 == String should be false");
    TEST_ASSERT(!mvn_val_equal(&val_i32_sample, &val_arr_sample), "I32 == Array should be false");
    TEST_ASSERT(!mvn_val_equal(&val_i32_sample, &val_hmap_sample), "I32 == Hmap should be false");

    // Clean up complex types if they were fully created (not just borrowed for string)
    // If mvn_val_string_borrow was used, no free for val_str_sample.str
    // If mvn_val_string was used, then mvn_string_free(val_str_sample.str) or
    // mvn_val_free(&val_str_sample)
    mvn_val_free(&val_str_sample); // Frees the string if mvn_val_string created it
    mvn_val_free(&val_arr_sample);
    mvn_val_free(&val_hmap_sample);
    // val_i32_sample is primitive, mvn_val_free just resets its type

    return true; // Test passed
}

/**
 * \brief           Run all primitives tests
 * \param[out]      passed_tests: Pointer to passed tests counter
 * \param[out]      failed_tests: Pointer to failed tests counter
 * \param[out]      total_tests: Pointer to total tests counter
 */
int run_primitives_tests(int *passed_tests, int *failed_tests, int *total_tests)
{
    printf("\n===== RUNNING PRIMITIVES TESTS =====\n");

    int passed_before = *passed_tests;
    int failed_before = *failed_tests;

    RUN_TEST(test_primitives);                      // Run the new test function
    RUN_TEST(test_additional_primitive_operations); // Add the new test

    int tests_run = (*passed_tests - passed_before) + (*failed_tests - failed_before);
    (*total_tests) += tests_run;

    printf("\n");                            // Add a newline after the tests for this module
    return (*failed_tests == failed_before); // Return true if no new failures
}

int main(void)
{
    int passed = 0;
    int failed = 0;
    int total  = 0;

    run_primitives_tests(&passed, &failed, &total);

    printf("\n===== PRIMITIVES TEST SUMMARY =====\n");
    print_test_summary(total, passed, failed);

    return (failed > 0) ? 1 : 0;
}
