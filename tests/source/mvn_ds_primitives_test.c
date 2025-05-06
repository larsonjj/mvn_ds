/*
 * Copyright (c) 2024 Jake Larson
 */
#include "mvn_ds_primitives_test.h"

#include "mvn_ds/mvn_ds.h"
#include "mvn_ds/mvn_ds_arr.h"  // For mvn_val_arr
#include "mvn_ds/mvn_ds_hmap.h" // For mvn_val_hmap
#include "mvn_ds/mvn_ds_str.h"  // For mvn_val_str_borrow
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

static bool test_primitive_equality_detailed(void)
{
    /*
     * Copyright (c) 2024 Jake Larson
     */
    mvn_val_t val_null1 = mvn_val_null();
    mvn_val_t val_null2 = mvn_val_null();

    mvn_val_t val_bool_t1 = mvn_val_bool(true);
    mvn_val_t val_bool_t2 = mvn_val_bool(true);
    mvn_val_t val_bool_f1 = mvn_val_bool(false);

    mvn_val_t val_i32_10  = mvn_val_i32(10);
    mvn_val_t val_i32_10b = mvn_val_i32(10);
    mvn_val_t val_i32_20  = mvn_val_i32(20);

    mvn_val_t val_i64_50  = mvn_val_i64(50LL);
    mvn_val_t val_i64_50b = mvn_val_i64(50LL);
    mvn_val_t val_i64_60  = mvn_val_i64(60LL);

    mvn_val_t val_f32_1p23  = mvn_val_f32(1.23f);
    mvn_val_t val_f32_1p23b = mvn_val_f32(1.23f);
    mvn_val_t val_f32_1p23e = mvn_val_f32(1.230000001f); // Within default epsilon for f32
    mvn_val_t val_f32_1p24  = mvn_val_f32(1.24f);

    mvn_val_t val_f64_3p1415  = mvn_val_f64(3.1415);
    mvn_val_t val_f64_3p1415b = mvn_val_f64(3.1415);
    mvn_val_t val_f64_3p1415e = mvn_val_f64(3.141500000000001); // Within default epsilon for f64
    mvn_val_t val_f64_3p1416  = mvn_val_f64(3.1416);

    // Same type, same value
    TEST_ASSERT(mvn_val_equal(&val_null1, &val_null2), "NULL == NULL failed");
    TEST_ASSERT(mvn_val_equal(&val_bool_t1, &val_bool_t2), "Bool(T) == Bool(T) failed");
    TEST_ASSERT(mvn_val_equal(&val_i32_10, &val_i32_10b), "I32(10) == I32(10) failed");
    TEST_ASSERT(mvn_val_equal(&val_i64_50, &val_i64_50b), "I64(50) == I64(50) failed");
    TEST_ASSERT(mvn_val_equal(&val_f32_1p23, &val_f32_1p23b), "F32(1.23) == F32(1.23) failed");
    TEST_ASSERT(mvn_val_equal(&val_f32_1p23, &val_f32_1p23e), "F32(1.23) == F32(1.23+eps) failed");
    TEST_ASSERT(mvn_val_equal(&val_f64_3p1415, &val_f64_3p1415b),
                "F64(3.1415) == F64(3.1415) failed");
    TEST_ASSERT(mvn_val_equal(&val_f64_3p1415, &val_f64_3p1415e),
                "F64(3.1415) == F64(3.1415+eps) failed");

    // Same type, different value
    TEST_ASSERT(!mvn_val_equal(&val_bool_t1, &val_bool_f1), "Bool(T) == Bool(F) reported true");
    TEST_ASSERT(!mvn_val_equal(&val_i32_10, &val_i32_20), "I32(10) == I32(20) reported true");
    TEST_ASSERT(!mvn_val_equal(&val_i64_50, &val_i64_60), "I64(50) == I64(60) reported true");
    TEST_ASSERT(!mvn_val_equal(&val_f32_1p23, &val_f32_1p24),
                "F32(1.23) == F32(1.24) reported true");
    TEST_ASSERT(!mvn_val_equal(&val_f64_3p1415, &val_f64_3p1416),
                "F64(3.1415) == F64(3.1416) reported true");

    // Different types
    TEST_ASSERT(!mvn_val_equal(&val_null1, &val_bool_t1), "NULL == Bool reported true");
    TEST_ASSERT(!mvn_val_equal(&val_bool_t1, &val_i32_10), "Bool == I32 reported true");
    TEST_ASSERT(!mvn_val_equal(&val_i32_10, &val_i64_50), "I32 == I64 reported true");
    TEST_ASSERT(!mvn_val_equal(&val_i64_50, &val_f32_1p23), "I64 == F32 reported true");
    TEST_ASSERT(!mvn_val_equal(&val_f32_1p23, &val_f64_3p1415), "F32 == F64 reported true");
    TEST_ASSERT(!mvn_val_equal(&val_i32_10, &val_f32_1p23), "I32 == F32 reported true");

    // Freeing primitives just resets their type, no actual memory deallocation for the value itself
    mvn_val_free(&val_null1);
    mvn_val_free(&val_null2);
    mvn_val_free(&val_bool_t1);
    mvn_val_free(&val_bool_t2);
    mvn_val_free(&val_bool_f1);
    mvn_val_free(&val_i32_10);
    mvn_val_free(&val_i32_10b);
    mvn_val_free(&val_i32_20);
    mvn_val_free(&val_i64_50);
    mvn_val_free(&val_i64_50b);
    mvn_val_free(&val_i64_60);
    mvn_val_free(&val_f32_1p23);
    mvn_val_free(&val_f32_1p23b);
    mvn_val_free(&val_f32_1p23e);
    mvn_val_free(&val_f32_1p24);
    mvn_val_free(&val_f64_3p1415);
    mvn_val_free(&val_f64_3p1415b);
    mvn_val_free(&val_f64_3p1415e);
    mvn_val_free(&val_f64_3p1416);

    return true;
}

static bool test_primitive_print(void)
{
    /*
     * Copyright (c) 2024 Jake Larson
     */
    printf("Testing mvn_val_print for all primitive types (visual check/no crash):\n");

    mvn_val_t val_null_p = mvn_val_null();
    printf("  Null: ");
    mvn_val_print(&val_null_p);
    printf("\n");

    mvn_val_t val_bool_t_p = mvn_val_bool(true);
    printf("  Bool(T): ");
    mvn_val_print(&val_bool_t_p);
    printf("\n");
    mvn_val_free(&val_bool_t_p);

    mvn_val_t val_bool_f_p = mvn_val_bool(false);
    printf("  Bool(F): ");
    mvn_val_print(&val_bool_f_p);
    printf("\n");
    mvn_val_free(&val_bool_f_p);

    mvn_val_t val_i32_p = mvn_val_i32(42);
    printf("  I32: ");
    mvn_val_print(&val_i32_p);
    printf("\n");
    mvn_val_free(&val_i32_p);

    mvn_val_t val_i64_p = mvn_val_i64(1234567890123LL);
    printf("  I64: ");
    mvn_val_print(&val_i64_p);
    printf("\n");
    mvn_val_free(&val_i64_p);

    mvn_val_t val_f32_p = mvn_val_f32(3.14f);
    printf("  F32: ");
    mvn_val_print(&val_f32_p);
    printf("\n");
    mvn_val_free(&val_f32_p);

    mvn_val_t val_f64_p = mvn_val_f64(2.718281828459);
    printf("  F64: ");
    mvn_val_print(&val_f64_p);
    printf("\n");
    mvn_val_free(&val_f64_p);

    printf("End of mvn_val_print primitive types test.\n");
    return true; // Test passed if no crashes
}

static bool test_val_type_to_string_conversion(void)
{
    /*
     * Copyright (c) 2024 Jake Larson
     */
    TEST_ASSERT(strcmp(mvn_val_type_to_str(MVN_VAL_NULL), "NULL") == 0,
                "MVN_VAL_NULL to string failed");
    TEST_ASSERT(strcmp(mvn_val_type_to_str(MVN_VAL_BOOL), "BOOL") == 0,
                "MVN_VAL_BOOL to string failed");
    TEST_ASSERT(strcmp(mvn_val_type_to_str(MVN_VAL_I32), "I32") == 0,
                "MVN_VAL_I32 to string failed");
    TEST_ASSERT(strcmp(mvn_val_type_to_str(MVN_VAL_I64), "I64") == 0,
                "MVN_VAL_I64 to string failed");
    TEST_ASSERT(strcmp(mvn_val_type_to_str(MVN_VAL_F32), "F32") == 0,
                "MVN_VAL_F32 to string failed");
    TEST_ASSERT(strcmp(mvn_val_type_to_str(MVN_VAL_F64), "F64") == 0,
                "MVN_VAL_F64 to string failed");
    TEST_ASSERT(strcmp(mvn_val_type_to_str(MVN_VAL_STRING), "STRING") == 0,
                "MVN_VAL_STRING to string failed");
    TEST_ASSERT(strcmp(mvn_val_type_to_str(MVN_VAL_ARRAY), "ARRAY") == 0,
                "MVN_VAL_ARRAY to string failed");
    TEST_ASSERT(strcmp(mvn_val_type_to_str(MVN_VAL_HASHMAP), "HASHMAP") == 0,
                "MVN_VAL_HASHMAP to string failed");
    // Test an out-of-range type, expecting "UNKNOWN"
    TEST_ASSERT(strcmp(mvn_val_type_to_str((mvn_val_type_t)999), "UNKNOWN") == 0,
                "Unknown type to string failed");

    return true;
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
    mvn_val_t val_str_sample  = mvn_val_str("sample"); // Changed from mvn_val_str_borrow
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
    // If mvn_val_str_borrow was used, no free for val_str_sample.str
    // If mvn_val_str was used, then mvn_string_free(val_str_sample.str) or
    // mvn_val_free(&val_str_sample)
    mvn_val_free(&val_str_sample); // Frees the string if mvn_val_str created it
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

    RUN_TEST(test_primitives);
    RUN_TEST(test_additional_primitive_operations);
    RUN_TEST(test_primitive_equality_detailed);   // Added
    RUN_TEST(test_primitive_print);               // Added
    RUN_TEST(test_val_type_to_string_conversion); // Added

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
