/*
 * Copyright (c) 2024 Jake Larson
 */
#include "mvn_ds_array_test.h"

#include "mvn_ds/mvn_ds.h"
#include "mvn_ds_test_utils.h"

#include <math.h>    // For fabsf, fabs
#include <stdbool.h> // Include for bool type
#include <stdio.h>
#include <string.h>

// --- Test Functions ---

static int test_array_creation_and_destruction(void)
{
    mvn_array_t *array = mvn_array_new();
    TEST_ASSERT(array != NULL, "Failed to create array");
    TEST_ASSERT(array->count == 0, "New array count should be 0");
    // Expect default initial capacity now
    TEST_ASSERT(array->capacity == MVN_DS_ARRAY_INITIAL_CAPACITY,
                "New array capacity should be MVN_INITIAL_CAPACITY");
    TEST_ASSERT(array->data != NULL, "New array data pointer should be non-NULL");

    mvn_array_free(array); // Should not crash

    // Test with specific initial capacity (e.g., 0)
    array = mvn_array_new_with_capacity(0);
    TEST_ASSERT(array != NULL, "Failed to create array with capacity 0");
    TEST_ASSERT(array->count == 0, "New array (cap 0) count should be 0");
    TEST_ASSERT(array->capacity == 0, "New array (cap 0) capacity should be 0");
    TEST_ASSERT(array->data == NULL, "New array (cap 0) data pointer should be NULL");
    mvn_array_free(array);

    // Test with specific initial capacity > 0
    array = mvn_array_new_with_capacity(10);
    TEST_ASSERT(array != NULL, "Failed to create array with capacity 10");
    TEST_ASSERT(array->count == 0, "New array (cap 10) count should be 0");
    TEST_ASSERT(array->capacity == 10, "New array (cap 10) capacity should be 10");
    TEST_ASSERT(array->data != NULL, "New array (cap 10) data pointer should be non-NULL");

    mvn_array_free(array); // Should not crash

    return true; // Test passed
}

static int test_array_push_and_get(void)
{
    mvn_array_t *array = mvn_array_new();
    TEST_ASSERT(array != NULL, "Failed to create array for push/get test");

    // Push various types
    bool push_ok = true;
    push_ok &= mvn_array_push(array, mvn_val_null());
    push_ok &= mvn_array_push(array, mvn_val_bool(true));
    push_ok &= mvn_array_push(array, mvn_val_i32(123));
    push_ok &= mvn_array_push(array, mvn_val_i64(4567890123LL));
    push_ok &= mvn_array_push(array, mvn_val_f32(3.14f));
    push_ok &= mvn_array_push(array, mvn_val_f64(2.71828));
    push_ok &= mvn_array_push(array, mvn_val_string("hello"));

    TEST_ASSERT(push_ok, "Failed to push one or more values");
    TEST_ASSERT(array->count == 7, "Array count should be 7 after pushes");

    // Get and verify types and values
    mvn_val_t *val = NULL;

    val = mvn_array_get(array, 0);
    TEST_ASSERT(val != NULL && val->type == MVN_VAL_NULL, "Index 0 should be NULL");

    val = mvn_array_get(array, 1);
    TEST_ASSERT(val != NULL && val->type == MVN_VAL_BOOL && val->b == true,
                "Index 1 should be true");

    val = mvn_array_get(array, 2);
    TEST_ASSERT(val != NULL && val->type == MVN_VAL_I32 && val->i32 == 123,
                "Index 2 should be i32(123)");

    val = mvn_array_get(array, 3);
    TEST_ASSERT(val != NULL && val->type == MVN_VAL_I64 && val->i64 == 4567890123LL,
                "Index 3 should be i64(4567890123)");

    val = mvn_array_get(array, 4);
    TEST_ASSERT_FLOAT_EQ(val->f32, 3.14f, "Index 4 should be f32(3.14)");

    val = mvn_array_get(array, 5);
    TEST_ASSERT_DOUBLE_EQ(val->f64, 2.71828, "Index 5 should be f64(2.71828)");

    val = mvn_array_get(array, 6);
    TEST_ASSERT(val != NULL && val->type == MVN_VAL_STRING && val->str != NULL &&
                    strcmp(val->str->data, "hello") == 0,
                "Index 6 should be string(\"hello\")");

    // Test out-of-bounds get
    val = mvn_array_get(array, 7);
    TEST_ASSERT(val == NULL, "Getting index out of bounds should return NULL");
    val = mvn_array_get(array, 100);
    TEST_ASSERT(val == NULL, "Getting large index out of bounds should return NULL");

    mvn_array_free(array);
    return true; // Test passed
}

static int test_array_set(void)
{
    mvn_array_t *array = mvn_array_new();
    TEST_ASSERT(array != NULL, "Failed to create array for set test");

    // Push initial values
    mvn_array_push(array, mvn_val_i32(1));
    mvn_array_push(array, mvn_val_string("original"));
    mvn_array_push(array, mvn_val_bool(false));
    TEST_ASSERT(array->count == 3, "Array count should be 3 initially");

    // Set value at index 1
    bool set_ok = mvn_array_set(array, 1, mvn_val_string("replaced"));
    TEST_ASSERT(set_ok, "mvn_array_set should return true for valid index");
    TEST_ASSERT(array->count == 3, "Array count should remain 3 after set");

    mvn_val_t *val = mvn_array_get(array, 1);
    TEST_ASSERT(val != NULL && val->type == MVN_VAL_STRING && val->str != NULL &&
                    strcmp(val->str->data, "replaced") == 0,
                "Index 1 should be updated string(\"replaced\")");

    // Set value at index 0
    set_ok = mvn_array_set(array, 0, mvn_val_f64(9.81));
    TEST_ASSERT(set_ok, "mvn_array_set should return true for index 0");
    val = mvn_array_get(array, 0);
    TEST_ASSERT(val != NULL && val->type == MVN_VAL_F64, "Index 0 type should be F64");
    TEST_ASSERT_DOUBLE_EQ(val->f64, 9.81, "Index 0 value should be 9.81");

    // Test setting out of bounds
    set_ok = mvn_array_set(array, 3, mvn_val_i32(999)); // Index 3 is out of bounds (count is 3)
    TEST_ASSERT(!set_ok, "mvn_array_set should return false for out-of-bounds index");
    TEST_ASSERT(array->count == 3, "Array count should remain 3 after failed set");

    // Verify other elements weren't disturbed by failed set
    val = mvn_array_get(array, 2);
    TEST_ASSERT(val != NULL && val->type == MVN_VAL_BOOL && val->b == false,
                "Index 2 should still be false");

    mvn_array_free(array);
    return true; // Test passed
}

static int test_array_resize(void)
{
    // Start with small capacity to force resize
    mvn_array_t *array = mvn_array_new_with_capacity(2);
    TEST_ASSERT(array != NULL, "Failed to create array for resize test");
    TEST_ASSERT(array->capacity == 2, "Initial capacity should be 2");

    // Push elements to trigger resize
    bool push_ok = true;
    push_ok &= mvn_array_push(array, mvn_val_i32(1));
    push_ok &= mvn_array_push(array, mvn_val_i32(2));
    TEST_ASSERT(array->count == 2, "Count should be 2 before resize");
    TEST_ASSERT(array->capacity == 2, "Capacity should be 2 before resize");

    push_ok &= mvn_array_push(array, mvn_val_i32(3)); // This should trigger resize
    TEST_ASSERT(push_ok, "Push triggering resize failed");
    TEST_ASSERT(array->count == 3, "Count should be 3 after resize");
    TEST_ASSERT(array->capacity > 2, "Capacity should be greater than 2 after resize");

    size_t capacity_after_first_resize = array->capacity;

    // Push more elements to potentially trigger another resize
    for (int i = 4; i <= 20; ++i) {
        push_ok &= mvn_array_push(array, mvn_val_i32(i));
    }
    TEST_ASSERT(push_ok, "Pushing multiple elements failed");
    TEST_ASSERT(array->count == 20, "Count should be 20 after multiple pushes");
    TEST_ASSERT(array->capacity >= 20, "Capacity should be at least 20");
    TEST_ASSERT(array->capacity > capacity_after_first_resize,
                "Capacity should have increased again");

    // Verify data integrity after resize
    for (int i = 0; i < 20; ++i) {
        mvn_val_t *val = mvn_array_get(array, i);
        TEST_ASSERT(val != NULL && val->type == MVN_VAL_I32 && val->i32 == (i + 1),
                    "Data verification failed after resize");
    }

    mvn_array_free(array);
    return true; // Test passed
}

static int test_array_ownership_free(void)
{
    mvn_array_t *array = mvn_array_new();
    TEST_ASSERT(array != NULL, "Failed to create array for ownership test");

    // Push dynamic types
    bool push_ok = true;
    push_ok &= mvn_array_push(array, mvn_val_string("string1"));
    push_ok &= mvn_array_push(array, mvn_val_array()); // Push an empty array
    push_ok &= mvn_array_push(array, mvn_val_hmap());  // Push an empty hmap
    push_ok &= mvn_array_push(array, mvn_val_string("string2"));

    // Add an element to the nested array to ensure deep free is tested
    mvn_val_t *nested_array_val = mvn_array_get(array, 1);
    TEST_ASSERT(nested_array_val != NULL && nested_array_val->type == MVN_VAL_ARRAY,
                "Failed to get nested array");
    if (nested_array_val && nested_array_val->type == MVN_VAL_ARRAY) {
        push_ok &= mvn_array_push(nested_array_val->arr, mvn_val_string("nested_string"));
    }

    TEST_ASSERT(push_ok, "Failed to push dynamic types for ownership test");
    TEST_ASSERT(array->count == 4, "Count should be 4");

    // Freeing the array should free all contained dynamic values recursively.
    // This test primarily relies on memory checking tools (like Valgrind or ASan)
    // to confirm no leaks occur.
    mvn_array_free(array);

    // Test freeing an array containing values taken via _take
    array                   = mvn_array_new();
    mvn_string_t *taken_str = mvn_string_new("taken");
    mvn_array_push(array, mvn_val_string_take(taken_str));
    mvn_array_free(array); // Should free the taken string

    return true; // Test passed (pending memory check)
}

static int test_array_ownership_set(void)
{
    mvn_array_t *array = mvn_array_new();
    TEST_ASSERT(array != NULL, "Failed to create array for set ownership test");

    // Push initial dynamic value
    mvn_array_push(array, mvn_val_string("original_string"));
    TEST_ASSERT(array->count == 1, "Count should be 1");

    // Set a new dynamic value over the old one
    // The old "original_string" should be freed by mvn_array_set.
    bool set_ok = mvn_array_set(array, 0, mvn_val_string("new_string"));
    TEST_ASSERT(set_ok, "Set with dynamic type failed");
    TEST_ASSERT(array->count == 1, "Count should remain 1 after set");
    mvn_val_t *val = mvn_array_get(array, 0);
    TEST_ASSERT(val != NULL && val->type == MVN_VAL_STRING && val->str != NULL &&
                    strcmp(val->str->data, "new_string") == 0,
                "Value mismatch after setting dynamic type");

    // Set a primitive value over the dynamic one
    // The "new_string" should be freed by mvn_array_set.
    set_ok = mvn_array_set(array, 0, mvn_val_i32(123));
    TEST_ASSERT(set_ok, "Set with primitive type failed");
    TEST_ASSERT(array->count == 1, "Count should remain 1 after set");
    val = mvn_array_get(array, 0);
    TEST_ASSERT(val != NULL && val->type == MVN_VAL_I32 && val->i32 == 123,
                "Value mismatch after setting primitive type");

    // Set a dynamic value using _take over the primitive
    mvn_string_t *taken_str = mvn_string_new("taken_set");
    set_ok                  = mvn_array_set(array, 0, mvn_val_string_take(taken_str));
    TEST_ASSERT(set_ok, "Set with taken string failed");
    val = mvn_array_get(array, 0);
    TEST_ASSERT(val != NULL && val->type == MVN_VAL_STRING && val->str == taken_str,
                "Value mismatch after setting taken string");

    // Freeing the array should free the last set value ("taken_set").
    // Again, relies on memory checking tools.
    mvn_array_free(array);

    return true; // Test passed (pending memory check)
}

/**
 * @brief Tests freeing a NULL array pointer.
 */
static bool test_array_free_null(void)
{
    mvn_array_free(NULL); // Should not crash or cause issues
    return true;          // Test passed if it doesn't crash
}

/**
 * @brief Tests pushing elements into an array starting with zero capacity.
 */
static bool test_array_resize_from_zero(void)
{
    mvn_array_t *zero_cap_array = mvn_array_new_with_capacity(0);
    TEST_ASSERT(zero_cap_array != NULL, "Failed to create zero-capacity array");
    TEST_ASSERT(zero_cap_array->count == 0, "Initial count should be 0");
    TEST_ASSERT(zero_cap_array->capacity == 0, "Initial capacity should be 0");
    TEST_ASSERT(zero_cap_array->data == NULL, "Initial data should be NULL");

    // First push should trigger allocation and resize
    bool push_ok = mvn_array_push(zero_cap_array, mvn_val_i32(100));
    TEST_ASSERT(push_ok, "Push into zero-capacity array failed");
    TEST_ASSERT(zero_cap_array->count == 1, "Count should be 1 after first push");
    TEST_ASSERT(zero_cap_array->capacity > 0, "Capacity should be > 0 after first push");
    TEST_ASSERT(zero_cap_array->data != NULL, "Data should not be NULL after first push");

    mvn_val_t *val = mvn_array_get(zero_cap_array, 0);
    TEST_ASSERT(val != NULL && val->type == MVN_VAL_I32 && val->i32 == 100,
                "Value verification failed after push to zero-cap array");

    mvn_array_free(zero_cap_array);
    return true; // Test passed
}

/**
 * @brief Tests mvn_array_set replacing primitive types.
 */
static bool test_array_set_primitive_replacement(void)
{
    mvn_array_t *array = mvn_array_new();
    TEST_ASSERT(array != NULL, "Failed to create array for primitive set test");

    // Push initial primitive values
    mvn_array_push(array, mvn_val_i32(1));
    mvn_array_push(array, mvn_val_f32(2.0f));
    TEST_ASSERT(array->count == 2, "Initial count should be 2");

    // 1. Set primitive over primitive
    bool set_ok = mvn_array_set(array, 0, mvn_val_bool(true));
    TEST_ASSERT(set_ok, "Set primitive over primitive failed");
    mvn_val_t *val = mvn_array_get(array, 0);
    TEST_ASSERT(val != NULL && val->type == MVN_VAL_BOOL && val->b == true,
                "Value mismatch after setting primitive over primitive");
    TEST_ASSERT(array->count == 2, "Count should remain 2"); // Ensure count doesn't change

    // 2. Set dynamic over primitive
    set_ok = mvn_array_set(array, 1, mvn_val_string("dynamic"));
    TEST_ASSERT(set_ok, "Set dynamic over primitive failed");
    val = mvn_array_get(array, 1);
    TEST_ASSERT(val != NULL && val->type == MVN_VAL_STRING && val->str != NULL &&
                    strcmp(val->str->data, "dynamic") == 0,
                "Value mismatch after setting dynamic over primitive");
    TEST_ASSERT(array->count == 2, "Count should remain 2");

    mvn_array_free(array);
    return true; // Test passed
}

/**
 * @brief Tests operations on a NULL array pointer.
 */
static bool test_array_null_param_safety(void)
{
    mvn_val_t *get_val = mvn_array_get(NULL, 0);
    TEST_ASSERT(get_val == NULL, "mvn_array_get(NULL, ...) should return NULL");

    bool set_ok = mvn_array_set(NULL, 0, mvn_val_i32(1));
    TEST_ASSERT(!set_ok, "mvn_array_set(NULL, ...) should return false");

    bool push_ok = mvn_array_push(NULL, mvn_val_i32(1));
    TEST_ASSERT(!push_ok, "mvn_array_push(NULL, ...) should return false");

    // mvn_array_free(NULL) is already tested in test_array_free_null

    return true; //    Test passed
}

/**
 * @brief Tests setting a NULL value over an existing dynamic value.
 */
static bool test_array_set_null_over_dynamic(void)
{
    mvn_array_t *array_ptr = mvn_array_new();
    TEST_ASSERT(array_ptr != NULL, "Failed to create array for set NULL over dynamic test");

    // Push a dynamic value (string)
    bool push_ok = mvn_array_push(array_ptr, mvn_val_string("to_be_freed"));
    TEST_ASSERT(push_ok, "Pushing initial string failed");
    TEST_ASSERT(array_ptr->count == 1, "Count should be 1 after push");

    // Set MVN_VAL_NULL over the string. The string should be freed.
    // This relies on memory checking tools (Valgrind/ASan) to confirm the free.
    bool set_ok = mvn_array_set(array_ptr, 0, mvn_val_null());
    TEST_ASSERT(set_ok, "Setting MVN_VAL_NULL over dynamic value failed");
    TEST_ASSERT(array_ptr->count == 1, "Count should remain 1 after setting NULL");

    mvn_val_t *val_after_set = mvn_array_get(array_ptr, 0);
    TEST_ASSERT(val_after_set != NULL && val_after_set->type == MVN_VAL_NULL,
                "Value at index 0 should be MVN_VAL_NULL after set");

    mvn_array_free(array_ptr);
    return true; //    Test passed (pending memory check for the freed string)
}

// --- Test Runner ---

/**
 * \brief           Run all array tests
 * \param[out]      passed_tests: Pointer to passed tests counter
 * \param[out]      failed_tests: Pointer to failed tests counter
 * \param[out]      total_tests: Pointer to total tests counter
 */
int run_array_tests(int *passed_tests, int *failed_tests, int *total_tests)
{
    printf("\n===== RUNNING ARRAY TESTS =====\n");

    int passed_before = *passed_tests;
    int failed_before = *failed_tests;

    RUN_TEST(test_array_creation_and_destruction);
    RUN_TEST(test_array_push_and_get);
    RUN_TEST(test_array_set);
    RUN_TEST(test_array_resize);
    RUN_TEST(test_array_ownership_free);
    RUN_TEST(test_array_ownership_set);
    RUN_TEST(test_array_free_null);
    RUN_TEST(test_array_resize_from_zero);
    RUN_TEST(test_array_set_primitive_replacement);
    RUN_TEST(test_array_null_param_safety);     // Added
    RUN_TEST(test_array_set_null_over_dynamic); // Added

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

    run_array_tests(&passed, &failed, &total);

    printf("\n===== ARRAY TEST SUMMARY =====\n");
    print_test_summary(total, passed, failed);

    return (failed > 0) ? 1 : 0;
}
