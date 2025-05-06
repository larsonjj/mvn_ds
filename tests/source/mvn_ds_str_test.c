/*
 * Copyright (c) 2024 Jake Larson
 */
#include "mvn_ds_str_test.h"

#include "mvn_ds/mvn_ds.h"
#include "mvn_ds_test_utils.h"

#include <limits.h> // For SIZE_MAX
#include <stdbool.h>
#include <stdint.h> // For uint32_t
#include <stdio.h>
#include <string.h> // For strcmp, strlen, strncmp

// --- Test Functions ---

static bool test_string_creation_and_destruction(void)
{
    // Test mvn_str_new
    mvn_str_t *str_one = mvn_str_new("hello");
    TEST_ASSERT(str_one != NULL, "Failed to create string with mvn_str_new");
    TEST_ASSERT(str_one->length == 5, "mvn_str_new length mismatch");
    TEST_ASSERT(str_one->capacity >= 5, "mvn_str_new capacity too small");
    TEST_ASSERT(str_one->data != NULL, "mvn_str_new data is NULL");
    TEST_ASSERT(strcmp(str_one->data, "hello") == 0, "mvn_str_new content mismatch");
    TEST_ASSERT(str_one->data[5] == '\0', "mvn_str_new not null-terminated");
    mvn_str_free(str_one); // Should not crash

    // Test mvn_str_new with empty string
    mvn_str_t *str_two = mvn_str_new("");
    TEST_ASSERT(str_two != NULL, "Failed to create empty string");
    TEST_ASSERT(str_two->length == 0, "Empty string length should be 0");
    TEST_ASSERT(str_two->capacity >= 0, "Empty string capacity should be >= 0");
    TEST_ASSERT(str_two->data != NULL, "Empty string data should not be NULL");
    TEST_ASSERT(str_two->data[0] == '\0', "Empty string not null-terminated");
    mvn_str_free(str_two);

    // Test mvn_str_new_with_capacity
    mvn_str_t *str_three = mvn_str_new_with_capacity(20);
    TEST_ASSERT(str_three != NULL, "Failed to create string with capacity");
    TEST_ASSERT(str_three->length == 0, "New string (cap) length should be 0");
    TEST_ASSERT(str_three->capacity == 20, "New string (cap) capacity mismatch");
    TEST_ASSERT(str_three->data != NULL, "New string (cap) data is NULL");
    TEST_ASSERT(str_three->data[0] == '\0', "New string (cap) not null-terminated");
    mvn_str_free(str_three);

    // Test mvn_str_free with NULL
    mvn_str_free(NULL); // Should not crash

    return true; // Test passed
}

static bool test_string_zero_capacity_creation(void)
{
    mvn_str_t *str_zero = mvn_str_new_with_capacity(0);
    TEST_ASSERT(str_zero != NULL, "Failed to create string with zero capacity");
    TEST_ASSERT(str_zero->length == 0, "Zero capacity string length should be 0");
    TEST_ASSERT(str_zero->capacity == 0, "Zero capacity string capacity should be 0");
    TEST_ASSERT(str_zero->data != NULL, "Zero capacity string data should not be NULL");
    TEST_ASSERT(str_zero->data[0] == '\0', "Zero capacity string should be null-terminated");

    // Append to trigger resize
    bool append_ok = mvn_str_append_cstr(str_zero, "test");
    TEST_ASSERT(append_ok, "Append to zero capacity string failed");
    TEST_ASSERT(str_zero->length == 4, "Length after append to zero capacity mismatch");
    TEST_ASSERT(str_zero->capacity >= 4, "Capacity after append to zero capacity too small");
    TEST_ASSERT(strcmp(str_zero->data, "test") == 0,
                "Content after append to zero capacity mismatch");

    mvn_str_free(str_zero);
    return true; // Test passed
}

static bool test_string_append(void)
{
    mvn_str_t *str_main = mvn_str_new("start");
    TEST_ASSERT(str_main != NULL, "Failed to create string for append test");

    // Append non-empty string
    bool append_ok = mvn_str_append_cstr(str_main, "_middle");
    TEST_ASSERT(append_ok, "Append '_middle' failed");
    TEST_ASSERT(str_main->length == 12, "Length after first append mismatch");
    TEST_ASSERT(str_main->capacity >= 12, "Capacity after first append too small");
    TEST_ASSERT(strcmp(str_main->data, "start_middle") == 0, "Content after first append mismatch");

    // Append another string (potentially causing resize)
    append_ok = mvn_str_append_cstr(str_main, "_end_long_enough_to_force_resize_maybe");
    TEST_ASSERT(append_ok, "Append long string failed");
    size_t expected_length = strlen("start_middle_end_long_enough_to_force_resize_maybe");
    TEST_ASSERT(str_main->length == expected_length, "Length after second append mismatch");
    TEST_ASSERT(str_main->capacity >= expected_length, "Capacity after second append too small");
    TEST_ASSERT(strcmp(str_main->data, "start_middle_end_long_enough_to_force_resize_maybe") == 0,
                "Content after second append mismatch");

    // Append empty string
    size_t length_before_empty_append = str_main->length;
    append_ok                         = mvn_str_append_cstr(str_main, "");
    TEST_ASSERT(append_ok, "Append empty string failed");
    TEST_ASSERT(str_main->length == length_before_empty_append,
                "Length changed after appending empty string");
    TEST_ASSERT(strcmp(str_main->data, "start_middle_end_long_enough_to_force_resize_maybe") == 0,
                "Content changed after appending empty string");

    mvn_str_free(str_main);

    // Test appending to empty string
    mvn_str_t *str_empty = mvn_str_new("");
    append_ok            = mvn_str_append_cstr(str_empty, "appended");
    TEST_ASSERT(append_ok, "Append to empty string failed");
    TEST_ASSERT(str_empty->length == 8, "Length after append to empty mismatch");
    TEST_ASSERT(strcmp(str_empty->data, "appended") == 0, "Content after append to empty mismatch");
    mvn_str_free(str_empty);

    return true; // Test passed
}

static bool test_string_append_mvn_str(void)
{
    mvn_str_t *str_dest  = mvn_str_new("Destination");
    mvn_str_t *str_src   = mvn_str_new("_Source");
    mvn_str_t *str_empty = mvn_str_new("");
    TEST_ASSERT(str_dest != NULL && str_src != NULL && str_empty != NULL,
                "Failed to create strings for append test");

    // Append non-empty to non-empty
    bool append_ok = mvn_str_append(str_dest, str_src);
    TEST_ASSERT(append_ok, "Append mvn_str failed");
    TEST_ASSERT(str_dest->length == 18, "Length mismatch after append mvn_str");
    TEST_ASSERT(strcmp(str_dest->data, "Destination_Source") == 0,
                "Content mismatch after append mvn_str");

    // Append empty to non-empty
    size_t length_before = str_dest->length;
    append_ok            = mvn_str_append(str_dest, str_empty);
    TEST_ASSERT(append_ok, "Append empty mvn_str failed");
    TEST_ASSERT(str_dest->length == length_before, "Length changed after appending empty mvn_str");
    TEST_ASSERT(strcmp(str_dest->data, "Destination_Source") == 0,
                "Content changed after appending empty mvn_str");

    // Append non-empty to empty
    append_ok = mvn_str_append(str_empty, str_src);
    TEST_ASSERT(append_ok, "Append to empty mvn_str failed");
    TEST_ASSERT(str_empty->length == 7, "Length mismatch after append to empty");
    TEST_ASSERT(strcmp(str_empty->data, "_Source") == 0, "Content mismatch after append to empty");

    // Append NULL string (should fail)
    append_ok = mvn_str_append(str_dest, NULL);
    TEST_ASSERT(!append_ok, "Append NULL mvn_str should fail");
    append_ok = mvn_str_append(NULL, str_src);
    TEST_ASSERT(!append_ok, "Append to NULL mvn_str should fail");

    mvn_str_free(str_dest);
    mvn_str_free(str_src);
    mvn_str_free(str_empty);
    return true; // Test passed
}

static bool test_string_append_null_cstr(void)
{
    mvn_str_t *str_dest = mvn_str_new("start");
    TEST_ASSERT(str_dest != NULL, "Failed to create string for append NULL test");

    size_t original_length = str_dest->length;
    bool   append_ok       = mvn_str_append_cstr(str_dest, NULL);
    // The function should handle NULL gracefully, likely returning false or doing nothing
    TEST_ASSERT(!append_ok, "Appending NULL cstr should return false");
    TEST_ASSERT(str_dest->length == original_length,
                "Length should not change after appending NULL cstr");
    TEST_ASSERT(strcmp(str_dest->data, "start") == 0,
                "Content should not change after appending NULL cstr");

    mvn_str_free(str_dest);
    return true; // Test passed
}

static bool test_string_equal(void)
{
    mvn_str_t *str_a1 = mvn_str_new("abc");
    mvn_str_t *str_a2 = mvn_str_new("abc");
    mvn_str_t *str_b  = mvn_str_new("def");
    mvn_str_t *str_c  = mvn_str_new("abcd");
    mvn_str_t *str_d  = mvn_str_new("");
    mvn_str_t *str_e  = mvn_str_new("");

    TEST_ASSERT(str_a1 != NULL && str_a2 != NULL && str_b != NULL && str_c != NULL &&
                    str_d != NULL && str_e != NULL,
                "Failed to create strings for equality test");

    // Basic equality
    TEST_ASSERT(mvn_str_equal(str_a1, str_a2), "Equal strings reported as unequal");
    TEST_ASSERT(mvn_str_equal(str_d, str_e), "Empty strings reported as unequal");

    // Basic inequality
    TEST_ASSERT(!mvn_str_equal(str_a1, str_b), "Unequal strings reported as equal");
    TEST_ASSERT(!mvn_str_equal(str_a1, str_c), "Substring reported as equal");
    TEST_ASSERT(!mvn_str_equal(str_c, str_a1), "Superstring reported as equal");
    TEST_ASSERT(!mvn_str_equal(str_a1, str_d), "String and empty string reported as equal");
    TEST_ASSERT(!mvn_str_equal(str_d, str_a1), "Empty string and string reported as equal");

    // NULL checks
    TEST_ASSERT(!mvn_str_equal(str_a1, NULL), "String and NULL reported as equal");
    TEST_ASSERT(!mvn_str_equal(NULL, str_a1), "NULL and string reported as equal");
    TEST_ASSERT(!mvn_str_equal(NULL, NULL), "NULL and NULL reported as equal (should be false)");

    // Self comparison
    TEST_ASSERT(mvn_str_equal(str_a1, str_a1), "String not equal to itself");
    TEST_ASSERT(mvn_str_equal(str_d, str_d), "Empty string not equal to itself");

    mvn_str_free(str_a1);
    mvn_str_free(str_a2);
    mvn_str_free(str_b);
    mvn_str_free(str_c);
    mvn_str_free(str_d);
    mvn_str_free(str_e);

    return true; // Test passed
}

static bool test_string_equal_cstr(void)
{
    mvn_str_t  *str_mvn    = mvn_str_new("compare_me");
    mvn_str_t  *str_empty  = mvn_str_new("");
    const char *cstr_equal = "compare_me";
    const char *cstr_diff  = "compare_you";
    const char *cstr_empty = "";

    TEST_ASSERT(str_mvn != NULL && str_empty != NULL,
                "Failed to create strings for cstr equal test");

    // Equal
    TEST_ASSERT(mvn_str_equal_cstr(str_mvn, cstr_equal), "Equal cstr reported as unequal");
    TEST_ASSERT(mvn_str_equal_cstr(str_empty, cstr_empty), "Empty cstr reported as unequal");

    // Unequal
    TEST_ASSERT(!mvn_str_equal_cstr(str_mvn, cstr_diff), "Unequal cstr reported as equal");
    TEST_ASSERT(!mvn_str_equal_cstr(str_mvn, cstr_empty),
                "String and empty cstr reported as equal");
    TEST_ASSERT(!mvn_str_equal_cstr(str_empty, cstr_equal),
                "Empty string and cstr reported as equal");

    // NULL checks
    TEST_ASSERT(!mvn_str_equal_cstr(str_mvn, NULL), "String and NULL cstr reported as equal");
    TEST_ASSERT(!mvn_str_equal_cstr(NULL, cstr_equal), "NULL string and cstr reported as equal");
    TEST_ASSERT(!mvn_str_equal_cstr(NULL, NULL), "NULL string and NULL cstr reported as equal");

    mvn_str_free(str_mvn);
    mvn_str_free(str_empty);
    return true; // Test passed
}

static bool test_string_resize(void)
{
    // Start with small capacity
    mvn_str_t *str_resize = mvn_str_new_with_capacity(4);
    TEST_ASSERT(str_resize != NULL, "Failed to create string for resize test");
    TEST_ASSERT(str_resize->capacity == 4, "Initial capacity mismatch");

    // Append to fill capacity exactly
    bool append_ok = mvn_str_append_cstr(str_resize, "1234");
    TEST_ASSERT(append_ok, "Append to fill capacity failed");
    TEST_ASSERT(str_resize->length == 4, "Length after filling capacity mismatch");
    TEST_ASSERT(str_resize->capacity == 4, "Capacity should not change yet");

    // Append one more character to trigger resize
    append_ok = mvn_str_append_cstr(str_resize, "5");
    TEST_ASSERT(append_ok, "Append to trigger resize failed");
    TEST_ASSERT(str_resize->length == 5, "Length after resize mismatch");
    // Capacity should grow (e.g., 4 * 2 = 8, or initial 8 if growth factor logic differs)
    TEST_ASSERT(str_resize->capacity >= 5, "Capacity did not increase after resize");
    TEST_ASSERT(strcmp(str_resize->data, "12345") == 0, "Content mismatch after resize");

    size_t capacity_after_first_resize = str_resize->capacity;

    // Append a much larger string to force further resize
    const char *long_append = "abcdefghijklmnopqrstuvwxyz";
    append_ok               = mvn_str_append_cstr(str_resize, long_append);
    TEST_ASSERT(append_ok, "Append long string after resize failed");
    size_t final_length = 5 + strlen(long_append);
    TEST_ASSERT(str_resize->length == final_length, "Length after second resize mismatch");
    TEST_ASSERT(str_resize->capacity >= final_length, "Capacity too small after second resize");
    TEST_ASSERT(str_resize->capacity > capacity_after_first_resize,
                "Capacity did not increase after second resize");
    TEST_ASSERT(strncmp(str_resize->data, "12345abcdefghijklmnopqrstuvwxyz", final_length) == 0,
                "Content mismatch after second resize");

    mvn_str_free(str_resize);
    return true; // Test passed
}

static bool test_string_hash(void)
{
    mvn_str_t *str_one   = mvn_str_new("hello world");
    mvn_str_t *str_two   = mvn_str_new("hello world");
    mvn_str_t *str_diff  = mvn_str_new("hello_world");
    mvn_str_t *str_empty = mvn_str_new("");

    TEST_ASSERT(str_one != NULL && str_two != NULL && str_diff != NULL && str_empty != NULL,
                "Failed to create strings for hash test");

    uint32_t hash_one   = mvn_str_hash(str_one);
    uint32_t hash_two   = mvn_str_hash(str_two);
    uint32_t hash_diff  = mvn_str_hash(str_diff);
    uint32_t hash_empty = mvn_str_hash(str_empty);
    uint32_t hash_null  = mvn_str_hash(NULL);

    // Basic consistency
    TEST_ASSERT(hash_one == hash_two, "Hashes of equal strings do not match");
    TEST_ASSERT(hash_one != hash_diff, "Hashes of different strings match");
    TEST_ASSERT(hash_one != hash_empty, "Hash of string matches hash of empty string");

    // Check non-zero hash for non-empty string (highly likely for FNV-1a)
    TEST_ASSERT(hash_one != 0, "Hash of non-empty string is zero");
    // FNV-1a hash of empty string is non-zero (offset basis)
    TEST_ASSERT(hash_empty != 0, "Hash of empty string is zero");

    // NULL handling
    TEST_ASSERT(hash_null == 0, "Hash of NULL string should be 0");

    mvn_str_free(str_one);
    mvn_str_free(str_two);
    mvn_str_free(str_diff);
    mvn_str_free(str_empty);
    return true; // Test passed
}

static bool test_string_val_integration(void)
{
    // Test mvn_val_str (creates and owns)
    mvn_val_t val_str1 = mvn_val_str("value_one");
    TEST_ASSERT(val_str1.type == MVN_VAL_STRING, "mvn_val_str type mismatch");
    TEST_ASSERT(val_str1.str != NULL, "mvn_val_str internal string is NULL");
    TEST_ASSERT(strcmp(val_str1.str->data, "value_one") == 0, "mvn_val_str content mismatch");
    mvn_val_free(&val_str1); // Should free the internal string
    TEST_ASSERT(val_str1.type == MVN_VAL_NULL, "mvn_val_free did not reset type for string");
    // Cannot check val_str1.str here as it's freed

    // Test mvn_val_str_take (takes ownership)
    mvn_str_t *raw_str = mvn_str_new("value_two");
    TEST_ASSERT(raw_str != NULL, "Failed to create raw string for take test");
    mvn_val_t val_str2 = mvn_val_str_take(raw_str);
    TEST_ASSERT(val_str2.type == MVN_VAL_STRING, "mvn_val_str_take type mismatch");
    TEST_ASSERT(val_str2.str == raw_str, "mvn_val_str_take pointer mismatch");
    TEST_ASSERT(strcmp(val_str2.str->data, "value_two") == 0, "mvn_val_str_take content mismatch");
    mvn_val_free(&val_str2); // Should free the taken string
    TEST_ASSERT(val_str2.type == MVN_VAL_NULL, "mvn_val_free did not reset type for taken string");
    // raw_str pointer is now dangling, do not use

    return true; // Test passed
}

static bool test_string_val_take_null(void)
{
    mvn_val_t val_null = mvn_val_str_take(NULL);
    TEST_ASSERT(val_null.type == MVN_VAL_NULL, "Taking NULL string should result in MVN_VAL_NULL");
    // No need to free val_null as it's NULL type

    return true; // Test passed
}

/**
 * @brief Tests mvn_str_new_with_capacity with extremely large capacity values.
 */
static bool test_string_new_capacity_overflow(void)
{
    /* Copyright (c) 2024 Jake Larson */
    mvn_str_t *str_overflow = NULL;

    // If capacity is SIZE_MAX, capacity + 1 (for null terminator) will overflow size_t.
    // The function should detect this (e.g., capacity >= SIZE_MAX) and return NULL.
    str_overflow = mvn_str_new_with_capacity(SIZE_MAX);
    TEST_ASSERT(
        str_overflow == NULL,
        "mvn_str_new_with_capacity(SIZE_MAX) should return NULL due to capacity + 1 overflow");

    // Test with a capacity that is the largest possible before (capacity + 1) overflows.
    // This is SIZE_MAX - 1. The allocation for data would be (SIZE_MAX - 1) + 1 = SIZE_MAX bytes.
    // This allocation might still fail due to system memory limits,
    // but it shouldn't be due to a size_t overflow in calculating allocation_size.
    if (SIZE_MAX > 0) { // Ensure SIZE_MAX - 1 is a valid positive number
        mvn_str_t *str_large_valid_capacity = mvn_str_new_with_capacity(SIZE_MAX - 1);
        if (str_large_valid_capacity != NULL) {
            // If this succeeds, it means an allocation of SIZE_MAX bytes was attempted and
            // succeeded.
            TEST_ASSERT(str_large_valid_capacity->capacity == SIZE_MAX - 1,
                        "Capacity mismatch for mvn_str_new_with_capacity(SIZE_MAX - 1)");
            TEST_ASSERT(str_large_valid_capacity->length == 0,
                        "Length should be 0 for mvn_str_new_with_capacity(SIZE_MAX - 1)");
            TEST_ASSERT(str_large_valid_capacity->data != NULL,
                        "Data should not be NULL for mvn_str_new_with_capacity(SIZE_MAX - 1)");
            TEST_ASSERT(
                str_large_valid_capacity->data[0] == '\0',
                "String should be null-terminated for mvn_str_new_with_capacity(SIZE_MAX - 1)");
            mvn_str_free(str_large_valid_capacity);
        } else {
            // This is an acceptable outcome if the system cannot allocate SIZE_MAX bytes.
            // The primary check is that it doesn't crash or misbehave due to overflow.
            printf("Note: mvn_str_new_with_capacity(SIZE_MAX - 1) returned NULL (likely out of "
                   "memory for %zu bytes).\n",
                   SIZE_MAX);
        }
    }
    return true; // Test passed
}

// --- Test Runner ---

/**
 * \brief           Run all string tests
 * \param[out]      passed_tests: Pointer to passed tests counter
 * \param[out]      failed_tests: Pointer to failed tests counter
 * \param[out]      total_tests: Pointer to total tests counter
 */
int run_string_tests(int *passed_tests, int *failed_tests, int *total_tests)
{
    printf("\n===== RUNNING STRING TESTS =====\n");

    int passed_before = *passed_tests;
    int failed_before = *failed_tests;

    RUN_TEST(test_string_creation_and_destruction);
    RUN_TEST(test_string_zero_capacity_creation); // Added
    RUN_TEST(test_string_append);
    RUN_TEST(test_string_append_mvn_str);
    RUN_TEST(test_string_append_null_cstr); // Added
    RUN_TEST(test_string_equal);
    RUN_TEST(test_string_equal_cstr);
    RUN_TEST(test_string_resize);
    RUN_TEST(test_string_hash);
    RUN_TEST(test_string_val_integration);
    RUN_TEST(test_string_val_take_null);         // Added
    RUN_TEST(test_string_new_capacity_overflow); // Added

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

    run_string_tests(&passed, &failed, &total);

    printf("\n===== STRING TEST SUMMARY =====\n");
    print_test_summary(total, passed, failed);

    return (failed > 0) ? 1 : 0;
}

/**
 * @brief Creates a new string with a specific initial capacity.
 * The allocated buffer will be capacity + 1 bytes for the null terminator.
 * The initial string content is empty ("").
 * @param capacity The initial capacity (excluding null terminator).
 * @return A pointer to the new mvn_str_t, or NULL on allocation failure or if capacity is too
 * large.
 */
mvn_str_t *mvn_str_new_with_capacity(size_t capacity)
{
    /* Copyright (c) 2024 Jake Larson */
    // Prevent capacity + 1 from overflowing or becoming SIZE_MAX.
    // If capacity is SIZE_MAX, capacity + 1 wraps to 0.
    // If capacity is SIZE_MAX - 1, capacity + 1 is SIZE_MAX.
    // Both are problematic: malloc(0) is implementation-defined, malloc(SIZE_MAX) is flagged by
    // Valgrind.
    if (capacity >= SIZE_MAX - 1) { // Check if capacity is SIZE_MAX or SIZE_MAX - 1
        fprintf(stderr,
                "[MVN_DS_STR] Requested capacity %zu is too large or would result in problematic "
                "allocation size.\n",
                capacity);
        return NULL;
    }

    mvn_str_t *string_ptr = (mvn_str_t *)MVN_DS_MALLOC(sizeof(mvn_str_t));
    if (!string_ptr) {
        return NULL; // Malloc failure for the struct itself
    }

    size_t allocation_size = capacity + 1;                           // For null terminator
    string_ptr->data       = (char *)MVN_DS_MALLOC(allocation_size); // This is line 103 from error
    if (!string_ptr->data) {
        MVN_DS_FREE(string_ptr);
        return NULL; // Malloc failure for the data buffer
    }

    string_ptr->length   = 0;
    string_ptr->capacity = capacity;
    string_ptr->data[0]  = '\0'; // Null-terminate the empty string

    return string_ptr;
}
