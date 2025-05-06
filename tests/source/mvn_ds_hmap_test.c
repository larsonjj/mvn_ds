/*
 * Copyright (c) 2024 Jake Larson
 */
#include "mvn_ds_hmap_test.h"

#include "mvn_ds/mvn_ds.h"
#include "mvn_ds_test_utils.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

// --- Defines ---
// Get the default initial capacity used in the source file
#ifndef MVN_INITIAL_CAPACITY
#define MVN_INITIAL_CAPACITY 8
#endif

// --- Test Functions ---

static bool test_hmap_creation_and_destruction(void)
{
    mvn_hmap_t *hmap = mvn_hmap_new();
    TEST_ASSERT(hmap != NULL, "Failed to create hash map");
    TEST_ASSERT(hmap->count == 0, "New hash map count should be 0");
    // Expect default initial capacity now
    TEST_ASSERT(hmap->capacity == MVN_DS_HMAP_INITIAL_CAPACITY,
                "New hash map capacity should be MVN_INITIAL_CAPACITY");
    TEST_ASSERT(hmap->buckets != NULL, "New hash map buckets should not be NULL");

    mvn_hmap_free(hmap); // Should not crash

    // Test with specific initial capacity (e.g., 0)
    hmap = mvn_hmap_new_with_capacity(0);
    TEST_ASSERT(hmap != NULL, "Failed to create hash map with capacity 0");
    TEST_ASSERT(hmap->count == 0, "New hash map (cap 0) count should be 0");
    TEST_ASSERT(hmap->capacity == 0, "New hash map (cap 0) capacity should be 0");
    TEST_ASSERT(hmap->buckets == NULL, "New hash map (cap 0) buckets should be NULL");
    mvn_hmap_free(hmap);

    // Test with specific initial capacity > 0
    hmap = mvn_hmap_new_with_capacity(32);
    TEST_ASSERT(hmap != NULL, "Failed to create hash map with capacity 32");
    TEST_ASSERT(hmap->count == 0, "New hash map (cap 32) count should be 0");
    TEST_ASSERT(hmap->capacity == 32, "New hash map (cap 32) capacity should be 32");
    TEST_ASSERT(hmap->buckets != NULL, "New hash map (cap 32) buckets should not be NULL");

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

// Helper to force collisions by controlling the hash result (for testing only)
// NOTE: This relies on internal knowledge/assumptions about mvn_string_hash.
// A more robust way might involve finding actual colliding strings or mocking.
// For simplicity, let's assume we can find strings or use a very small map.
static bool test_hmap_collisions(void)
{
    // Use a small capacity to increase likelihood/ease of collisions
    mvn_hmap_t *hmap = mvn_hmap_new_with_capacity(2);
    TEST_ASSERT(hmap != NULL, "Failed to create hash map for collision test");

    // Assume "keyA" and "keyC" collide in a map of capacity 2 (hash % 2 is same)
    // Assume "keyB" hashes to the other bucket.
    // These are assumptions for the test; real collisions depend on the hash function.
    // You might need to find actual colliding strings for your hash function.
    const char *key_a_str = "keyA"; // e.g., hash(...) % 2 == 0
    const char *key_b_str = "keyB"; // e.g., hash(...) % 2 == 1
    const char *key_c_str = "keyC"; // e.g., hash(...) % 2 == 0

    bool set_ok = true;
    set_ok &= mvn_hmap_set_cstr(hmap, key_a_str, mvn_val_i32(1));
    set_ok &= mvn_hmap_set_cstr(hmap, key_b_str, mvn_val_i32(2));
    set_ok &= mvn_hmap_set_cstr(hmap, key_c_str, mvn_val_i32(3)); // Collides with keyA

    TEST_ASSERT(set_ok, "Failed to set colliding keys");
    TEST_ASSERT(hmap->count == 3, "Count should be 3 after setting colliding keys");
    // Capacity might have grown due to resize logic during set
    TEST_ASSERT(hmap->capacity >= 2, "Capacity check after collision");

    // Verify all keys can be retrieved
    mvn_val_t *val_a = mvn_hmap_get_cstr(hmap, key_a_str);
    mvn_val_t *val_b = mvn_hmap_get_cstr(hmap, key_b_str);
    mvn_val_t *val_c = mvn_hmap_get_cstr(hmap, key_c_str);

    TEST_ASSERT(val_a != NULL && val_a->type == MVN_VAL_I32 && val_a->i32 == 1,
                "Failed to get keyA after collision");
    TEST_ASSERT(val_b != NULL && val_b->type == MVN_VAL_I32 && val_b->i32 == 2,
                "Failed to get keyB after collision");
    TEST_ASSERT(val_c != NULL && val_c->type == MVN_VAL_I32 && val_c->i32 == 3,
                "Failed to get keyC after collision");

    // Delete one of the colliding keys (e.g., keyA)
    bool delete_ok = mvn_hmap_delete_cstr(hmap, key_a_str);
    TEST_ASSERT(delete_ok, "Failed to delete keyA in collision chain");
    TEST_ASSERT(hmap->count == 2, "Count should be 2 after deleting keyA");

    // Verify keyA is gone, but keyB and keyC remain
    val_a = mvn_hmap_get_cstr(hmap, key_a_str);
    val_b = mvn_hmap_get_cstr(hmap, key_b_str);
    val_c = mvn_hmap_get_cstr(hmap, key_c_str);
    TEST_ASSERT(val_a == NULL, "keyA should be NULL after delete");
    TEST_ASSERT(val_b != NULL && val_b->i32 == 2, "keyB retrieval failed after deleting keyA");
    TEST_ASSERT(val_c != NULL && val_c->i32 == 3, "keyC retrieval failed after deleting keyA");

    // Delete the other colliding key (keyC)
    delete_ok = mvn_hmap_delete_cstr(hmap, key_c_str);
    TEST_ASSERT(delete_ok, "Failed to delete keyC in collision chain");
    TEST_ASSERT(hmap->count == 1, "Count should be 1 after deleting keyC");
    val_c = mvn_hmap_get_cstr(hmap, key_c_str);
    TEST_ASSERT(val_c == NULL, "keyC should be NULL after delete");
    val_b = mvn_hmap_get_cstr(hmap, key_b_str); // Verify keyB still exists
    TEST_ASSERT(val_b != NULL && val_b->i32 == 2, "keyB retrieval failed after deleting keyC");

    mvn_hmap_free(hmap);
    return true; // Test passed
}

static bool test_hmap_mvn_string_keys(void)
{
    mvn_hmap_t *hmap = mvn_hmap_new();
    TEST_ASSERT(hmap != NULL, "Failed to create hash map for mvn_string key test");

    // Create keys as mvn_string_t
    mvn_string_t *key_one     = mvn_string_new("key_one_str");
    mvn_string_t *key_two     = mvn_string_new("key_two_str");
    mvn_string_t *key_one_dup = mvn_string_new("key_one_str"); // Same content, different object

    TEST_ASSERT(key_one != NULL && key_two != NULL && key_one_dup != NULL,
                "Failed to create mvn_string keys");

    // Set using mvn_hmap_set (takes ownership of key_one and key_two)
    bool set_ok = true;
    set_ok &= mvn_hmap_set(hmap, key_one, mvn_val_i32(111));
    set_ok &= mvn_hmap_set(hmap, key_two, mvn_val_f32(2.22f));

    TEST_ASSERT(set_ok, "Failed to set using mvn_hmap_set");
    TEST_ASSERT(hmap->count == 2, "Count should be 2 after mvn_hmap_set");

    // Get using mvn_hmap_get (use key_one_dup to check content equality works)
    mvn_val_t *val_one = mvn_hmap_get(hmap, key_one_dup);
    mvn_val_t *val_two = mvn_hmap_get(hmap, key_two);

    TEST_ASSERT(val_one != NULL && val_one->type == MVN_VAL_I32 && val_one->i32 == 111,
                "Get key_one using mvn_hmap_get failed");
    TEST_ASSERT(val_two != NULL && val_two->type == MVN_VAL_F32,
                "Get key_two using mvn_hmap_get failed (type)");
    TEST_ASSERT_FLOAT_EQ(val_two->f32, 2.22f, "Get key_two using mvn_hmap_get failed (value)");

    // Try to set using a key that already exists (key_one_dup)
    // mvn_hmap_set should free the provided key_one_dup because the map keeps its original key.
    set_ok = mvn_hmap_set(hmap, key_one_dup, mvn_val_bool(false));
    TEST_ASSERT(set_ok, "Replacing value using mvn_hmap_set failed");
    TEST_ASSERT(hmap->count == 2, "Count should remain 2 after replace with mvn_hmap_set");
    // key_one_dup is now freed by mvn_hmap_set, do not use it further.

    // Verify the value was updated
    val_one = mvn_hmap_get(hmap, key_one); // Use original key pointer still held by map
    TEST_ASSERT(val_one != NULL && val_one->type == MVN_VAL_BOOL && val_one->b == false,
                "Value not updated after replace with mvn_hmap_set");

    // Delete using mvn_hmap_delete
    mvn_string_t *key_two_lookup = mvn_string_new("key_two_str"); // Create another lookup key
    TEST_ASSERT(key_two_lookup != NULL, "Failed to create lookup key for delete"); // Add check
    bool delete_ok = mvn_hmap_delete(hmap, key_two_lookup);
    TEST_ASSERT(delete_ok, "Delete using mvn_hmap_delete failed");
    TEST_ASSERT(hmap->count == 1, "Count should be 1 after mvn_hmap_delete");
    // val_two = mvn_hmap_get(hmap, key_two); // REMOVED: key_two pointer is invalid now
    // TEST_ASSERT(val_two == NULL, "Value should be NULL after mvn_hmap_delete"); // REMOVED

    mvn_string_free(key_two_lookup); // Must free the lookup key manually
    // key_one, key_two, key_one_dup were handled by the map (set/delete)

    mvn_hmap_free(hmap); // Frees the remaining key ("key_one_str") and its value
    return true;         // Test passed
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
    RUN_TEST(test_hmap_collisions);
    RUN_TEST(test_hmap_mvn_string_keys);

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
