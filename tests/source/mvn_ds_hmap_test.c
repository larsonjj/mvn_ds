/*
 * Copyright (c) 2024 Jake Larson
 */
#include "mvn_ds_hmap_test.h"

#include "mvn_ds/mvn_ds.h"
#include "mvn_ds_test_utils.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

// --- Test Functions ---

static bool test_hmap_creation_and_destruction(void)
{
    mvn_hmap_t *hmap = mvn_hmap_new();
    TEST_ASSERT(hmap != NULL, "Failed to create hash map");
    TEST_ASSERT(hmap->count == 0, "New hash map count should be 0");
    TEST_ASSERT(hmap->capacity > 0, "New hash map capacity should be > 0");
    TEST_ASSERT(hmap->buckets != NULL, "New hash map buckets should not be NULL");

    mvn_hmap_free(hmap); // Should not crash

    // Test with initial capacity
    hmap = mvn_hmap_new_with_capacity(32);
    TEST_ASSERT(hmap != NULL, "Failed to create hash map with capacity");
    TEST_ASSERT(hmap->count == 0, "New hash map (cap) count should be 0");
    TEST_ASSERT(hmap->capacity == 32, "New hash map (cap) capacity should be 32");
    TEST_ASSERT(hmap->buckets != NULL, "New hash map (cap) buckets should not be NULL");

    mvn_hmap_free(hmap); // Should not crash

    return true; // Test passed
}

static bool test_hmap_set_get_basic(void)
{
    mvn_hmap_t *hmap = mvn_hmap_new();
    TEST_ASSERT(hmap != NULL, "Failed to create hash map for set/get test");

    bool set_ok = true;
    // Use mvn_hmap_set_cstr for convenience
    set_ok &= mvn_hmap_set_cstr(hmap, "key1", mvn_val_i32(100));
    set_ok &= mvn_hmap_set_cstr(hmap, "key2", mvn_val_string("value2"));
    set_ok &= mvn_hmap_set_cstr(hmap, "enabled", mvn_val_bool(true));
    set_ok &= mvn_hmap_set_cstr(hmap, "pi", mvn_val_f64(3.14159));

    TEST_ASSERT(set_ok, "Failed to set one or more values");
    TEST_ASSERT(hmap->count == 4, "Hash map count should be 4 after sets");

    // Get and verify values using mvn_hmap_get_cstr
    mvn_val_t *val = NULL;

    val = mvn_hmap_get_cstr(hmap, "key1");
    TEST_ASSERT(val != NULL && val->type == MVN_VAL_I32 && val->i32 == 100,
                "Get 'key1' failed or value mismatch");

    val = mvn_hmap_get_cstr(hmap, "key2");
    TEST_ASSERT(val != NULL && val->type == MVN_VAL_STRING && val->str != NULL &&
                    strcmp(val->str->data, "value2") == 0,
                "Get 'key2' failed or value mismatch");

    val = mvn_hmap_get_cstr(hmap, "enabled");
    TEST_ASSERT(val != NULL && val->type == MVN_VAL_BOOL && val->b == true,
                "Get 'enabled' failed or value mismatch");

    val = mvn_hmap_get_cstr(hmap, "pi");
    TEST_ASSERT(val != NULL && val->type == MVN_VAL_F64, "Get 'pi' failed or type mismatch");
    TEST_ASSERT_DOUBLE_EQ(val->f64, 3.14159, "Get 'pi' value mismatch");

    // Test getting a non-existent key
    val = mvn_hmap_get_cstr(hmap, "nonexistent");
    TEST_ASSERT(val == NULL, "Getting non-existent key should return NULL");

    mvn_hmap_free(hmap);
    return true; // Test passed
}

static bool test_hmap_set_replace(void)
{
    mvn_hmap_t *hmap = mvn_hmap_new();
    TEST_ASSERT(hmap != NULL, "Failed to create hash map for replace test");

    mvn_hmap_set_cstr(hmap, "mykey", mvn_val_i32(1));
    TEST_ASSERT(hmap->count == 1, "Count should be 1 initially");

    mvn_val_t *val = mvn_hmap_get_cstr(hmap, "mykey");
    TEST_ASSERT(val != NULL && val->type == MVN_VAL_I32 && val->i32 == 1, "Initial get failed");

    // Replace the value with a different type
    bool set_ok = mvn_hmap_set_cstr(hmap, "mykey", mvn_val_string("new_value"));
    TEST_ASSERT(set_ok, "Replacing value failed");
    TEST_ASSERT(hmap->count == 1, "Count should remain 1 after replace");

    val = mvn_hmap_get_cstr(hmap, "mykey");
    TEST_ASSERT(val != NULL && val->type == MVN_VAL_STRING && val->str != NULL &&
                    strcmp(val->str->data, "new_value") == 0,
                "Get after replace failed or value mismatch");

    mvn_hmap_free(hmap);
    return true; // Test passed
}

static bool test_hmap_delete(void)
{
    mvn_hmap_t *hmap = mvn_hmap_new();
    TEST_ASSERT(hmap != NULL, "Failed to create hash map for delete test");

    mvn_hmap_set_cstr(hmap, "key_to_delete", mvn_val_i32(42));
    mvn_hmap_set_cstr(hmap, "key_to_keep", mvn_val_string("persistent"));
    TEST_ASSERT(hmap->count == 2, "Count should be 2 before delete");

    // Delete existing key
    bool delete_ok = mvn_hmap_delete_cstr(hmap, "key_to_delete");
    TEST_ASSERT(delete_ok, "Deleting existing key failed");
    TEST_ASSERT(hmap->count == 1, "Count should be 1 after delete");

    mvn_val_t *val = mvn_hmap_get_cstr(hmap, "key_to_delete");
    TEST_ASSERT(val == NULL, "Getting deleted key should return NULL");

    // Verify other key is still present
    val = mvn_hmap_get_cstr(hmap, "key_to_keep");
    TEST_ASSERT(val != NULL && val->type == MVN_VAL_STRING, "Other key was affected by delete");

    // Delete non-existent key
    delete_ok = mvn_hmap_delete_cstr(hmap, "nonexistent");
    TEST_ASSERT(!delete_ok, "Deleting non-existent key should return false");
    TEST_ASSERT(hmap->count == 1, "Count should remain 1 after failed delete");

    mvn_hmap_free(hmap);
    return true; // Test passed
}

static bool test_hmap_resize(void)
{
    // Start with small capacity to force resize
    mvn_hmap_t *hmap = mvn_hmap_new_with_capacity(2);
    TEST_ASSERT(hmap != NULL, "Failed to create hash map for resize test");
    TEST_ASSERT(hmap->capacity == 2, "Initial capacity should be 2");

    // Insert elements to trigger resize (load factor 0.75)
    bool set_ok = true;
    set_ok &= mvn_hmap_set_cstr(hmap, "key1", mvn_val_i32(1));
    TEST_ASSERT(hmap->count == 1, "Count should be 1");
    TEST_ASSERT(hmap->capacity == 2, "Capacity should be 2");

    set_ok &= mvn_hmap_set_cstr(hmap, "key2", mvn_val_i32(2)); // Should trigger resize
    TEST_ASSERT(set_ok, "Set triggering resize failed");
    TEST_ASSERT(hmap->count == 2, "Count should be 2 after resize");
    TEST_ASSERT(hmap->capacity > 2, "Capacity should increase after resize");

    size_t capacity_after_resize = hmap->capacity;

    // Verify data integrity after resize
    mvn_val_t *val1 = mvn_hmap_get_cstr(hmap, "key1");
    mvn_val_t *val2 = mvn_hmap_get_cstr(hmap, "key2");
    TEST_ASSERT(val1 != NULL && val1->type == MVN_VAL_I32 && val1->i32 == 1,
                "Value 'key1' incorrect after resize");
    TEST_ASSERT(val2 != NULL && val2->type == MVN_VAL_I32 && val2->i32 == 2,
                "Value 'key2' incorrect after resize");

    // Add more elements to potentially trigger another resize
    char key_buffer[16];
    for (int i = 3; i <= 10; ++i) {
        snprintf(key_buffer, sizeof(key_buffer), "key%d", i);
        set_ok &= mvn_hmap_set_cstr(hmap, key_buffer, mvn_val_i32(i));
    }
    TEST_ASSERT(set_ok, "Setting multiple elements failed");
    TEST_ASSERT(hmap->count == 10, "Count should be 10 after multiple sets");
    TEST_ASSERT(hmap->capacity >= capacity_after_resize, "Capacity should not decrease");

    // Verify all data again
    for (int i = 1; i <= 10; ++i) {
        snprintf(key_buffer, sizeof(key_buffer), "key%d", i);
        mvn_val_t *val = mvn_hmap_get_cstr(hmap, key_buffer);
        TEST_ASSERT(val != NULL && val->type == MVN_VAL_I32 && val->i32 == i,
                    "Data verification failed after multiple resizes");
    }

    mvn_hmap_free(hmap);
    return true; // Test passed
}

static bool test_hmap_ownership(void)
{
    mvn_hmap_t *hmap = mvn_hmap_new();
    TEST_ASSERT(hmap != NULL, "Failed to create hash map for ownership test");

    // Create nested structures
    mvn_val_t inner_array_val = mvn_val_array();
    mvn_array_push(inner_array_val.arr, mvn_val_i32(1));
    mvn_array_push(inner_array_val.arr, mvn_val_string("nested_string"));

    mvn_val_t inner_hmap_val = mvn_val_hmap();
    mvn_hmap_set_cstr(inner_hmap_val.hmap, "inner_key", mvn_val_bool(false));

    // Set values in the main hash map (ownership transferred)
    mvn_hmap_set_cstr(hmap, "outer_string", mvn_val_string("top_level"));
    mvn_hmap_set_cstr(hmap, "outer_array", inner_array_val);
    mvn_hmap_set_cstr(hmap, "outer_hmap", inner_hmap_val);

    TEST_ASSERT(hmap->count == 3, "Count should be 3 after setting nested structures");

    // Freeing the outer hash map should free all owned nested structures
    mvn_hmap_free(hmap);
    // If this doesn't crash or leak (checked with Valgrind/ASan), ownership works.

    // Test replacement ownership
    hmap = mvn_hmap_new();
    mvn_hmap_set_cstr(hmap, "replace_me", mvn_val_string("initial"));
    // This should free the "initial" string
    mvn_hmap_set_cstr(hmap, "replace_me", mvn_val_i32(99));
    mvn_hmap_free(hmap);

    // Test delete ownership
    hmap = mvn_hmap_new();
    mvn_hmap_set_cstr(hmap, "delete_me", mvn_val_array());
    mvn_array_push(mvn_hmap_get_cstr(hmap, "delete_me")->arr, mvn_val_i32(1));
    // This should free the array and its contents
    mvn_hmap_delete_cstr(hmap, "delete_me");
    mvn_hmap_free(hmap);

    return true; // Test passed (relies on memory checking tools for full verification)
}

// --- Test Runner ---

/**
 * \brief           Run all hmap tests
 * \param[out]      passed_tests: Pointer to passed tests counter
 * \param[out]      failed_tests: Pointer to failed tests counter
 * \param[out]      total_tests: Pointer to total tests counter
 */
int run_hmap_tests(int *passed_tests, int *failed_tests, int *total_tests)
{
    printf("\n===== RUNNING HASHMAP TESTS =====\n");

    int passed_before = *passed_tests;
    int failed_before = *failed_tests;

    RUN_TEST(test_hmap_creation_and_destruction);
    RUN_TEST(test_hmap_set_get_basic);
    RUN_TEST(test_hmap_set_replace);
    RUN_TEST(test_hmap_delete);
    RUN_TEST(test_hmap_resize);
    RUN_TEST(test_hmap_ownership);

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

    run_hmap_tests(&passed, &failed, &total);

    printf("\n===== HASHMAP TEST SUMMARY =====\n");
    print_test_summary(total, passed, failed);

    return (failed > 0) ? 1 : 0;
}
