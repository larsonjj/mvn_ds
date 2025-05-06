/*
 * Copyright (c) 2024 Jake Larson
 */
#include "mvn_ds_hmap_test.h"

#include "mvn_ds/mvn_ds.h"
#include "mvn_ds_test_utils.h"

#include <limits.h> // For SIZE_MAX
#include <math.h>
#include <stdbool.h> // For bool type
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
    hmap = mvn_hmap_new_capacity(0);
    TEST_ASSERT(hmap != NULL, "Failed to create hash map with capacity 0");
    TEST_ASSERT(hmap->count == 0, "New hash map (cap 0) count should be 0");
    TEST_ASSERT(hmap->capacity == 0, "New hash map (cap 0) capacity should be 0");
    TEST_ASSERT(hmap->buckets == NULL, "New hash map (cap 0) buckets should be NULL");
    mvn_hmap_free(hmap);

    // Test with specific initial capacity > 0
    hmap = mvn_hmap_new_capacity(32);
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
    set_ok &= mvn_hmap_set_cstr(hmap, "key2", mvn_val_str("value2"));
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
    bool set_ok = mvn_hmap_set_cstr(hmap, "mykey", mvn_val_str("new_value"));
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
    mvn_hmap_set_cstr(hmap, "key_to_keep", mvn_val_str("persistent"));
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
    mvn_hmap_t *hmap = mvn_hmap_new_capacity(2);
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
    mvn_val_t inner_array_val = mvn_val_arr();
    mvn_arr_push(inner_array_val.arr, mvn_val_i32(1));
    mvn_arr_push(inner_array_val.arr, mvn_val_str("nested_string"));

    mvn_val_t inner_hmap_val = mvn_val_hmap();
    mvn_hmap_set_cstr(inner_hmap_val.hmap, "inner_key", mvn_val_bool(false));

    // Set values in the main hash map (ownership transferred)
    mvn_hmap_set_cstr(hmap, "outer_string", mvn_val_str("top_level"));
    mvn_hmap_set_cstr(hmap, "outer_array", inner_array_val);
    mvn_hmap_set_cstr(hmap, "outer_hmap", inner_hmap_val);

    TEST_ASSERT(hmap->count == 3, "Count should be 3 after setting nested structures");

    // Freeing the outer hash map should free all owned nested structures
    mvn_hmap_free(hmap);
    // If this doesn't crash or leak (checked with Valgrind/ASan), ownership works.

    // Test replacement ownership
    hmap = mvn_hmap_new();
    mvn_hmap_set_cstr(hmap, "replace_me", mvn_val_str("initial"));
    // This should free the "initial" string
    mvn_hmap_set_cstr(hmap, "replace_me", mvn_val_i32(99));
    mvn_hmap_free(hmap);

    // Test delete ownership
    hmap = mvn_hmap_new();
    mvn_hmap_set_cstr(hmap, "delete_me", mvn_val_arr());
    mvn_arr_push(mvn_hmap_get_cstr(hmap, "delete_me")->arr, mvn_val_i32(1));
    // This should free the array and its contents
    mvn_hmap_delete_cstr(hmap, "delete_me");
    mvn_hmap_free(hmap);

    return true; // Test passed (relies on memory checking tools for full verification)
}

// Helper to force collisions by controlling the hash result (for testing only)
// NOTE: This relies on internal knowledge/assumptions about mvn_str_hash.
// A more robust way might involve finding actual colliding strings or mocking.
// For simplicity, let's assume we can find strings or use a very small map.
static bool test_hmap_collisions(void)
{
    // Use a small capacity to increase likelihood/ease of collisions
    mvn_hmap_t *hmap = mvn_hmap_new_capacity(2);
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

static bool test_hmap_mvn_str_keys(void)
{
    mvn_hmap_t *hmap = mvn_hmap_new();
    TEST_ASSERT(hmap != NULL, "Failed to create hash map for mvn_str key test");

    // Create keys as mvn_str_t
    mvn_str_t *key_one     = mvn_str_new("key_one_str");
    mvn_str_t *key_two     = mvn_str_new("key_two_str");
    mvn_str_t *key_one_dup = mvn_str_new("key_one_str"); // Same content, different object

    TEST_ASSERT(key_one != NULL && key_two != NULL && key_one_dup != NULL,
                "Failed to create mvn_str keys");

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
    mvn_str_t *key_two_lookup = mvn_str_new("key_two_str"); // Create another lookup key
    TEST_ASSERT(key_two_lookup != NULL, "Failed to create lookup key for delete"); // Add check
    bool delete_ok = mvn_hmap_delete(hmap, key_two_lookup);
    TEST_ASSERT(delete_ok, "Delete using mvn_hmap_delete failed");
    TEST_ASSERT(hmap->count == 1, "Count should be 1 after mvn_hmap_delete");
    // val_two = mvn_hmap_get(hmap, key_two); // REMOVED: key_two pointer is invalid now
    // TEST_ASSERT(val_two == NULL, "Value should be NULL after mvn_hmap_delete"); // REMOVED

    mvn_str_free(key_two_lookup); // Must free the lookup key manually
    // key_one, key_two, key_one_dup were handled by the map (set/delete)

    mvn_hmap_free(hmap); // Frees the remaining key ("key_one_str") and its value
    return true;         // Test passed
}

/**
 * @brief Tests freeing a NULL hash map pointer.
 */
static bool test_hmap_free_null(void)
{
    mvn_hmap_free(NULL); // Should not crash or cause issues
    return true;         // Test passed if it doesn't crash
}

/**
 * @brief Tests setting, getting, and deleting using an empty string key.
 */
static bool test_hmap_empty_string_key(void)
{
    mvn_hmap_t *hmap = mvn_hmap_new();
    TEST_ASSERT(hmap != NULL, "Failed to create hash map for empty string key test");

    // Set with empty string key
    bool set_ok = mvn_hmap_set_cstr(hmap, "", mvn_val_i32(123));
    TEST_ASSERT(set_ok, "Setting with empty string key failed");
    TEST_ASSERT(hmap->count == 1, "Count should be 1 after setting empty string key");

    // Get with empty string key
    mvn_val_t *val = mvn_hmap_get_cstr(hmap, "");
    TEST_ASSERT(val != NULL && val->type == MVN_VAL_I32 && val->i32 == 123,
                "Getting with empty string key failed or value mismatch");

    // Set another key to ensure delete doesn't break things
    set_ok = mvn_hmap_set_cstr(hmap, "other", mvn_val_bool(true));
    TEST_ASSERT(set_ok, "Setting 'other' key failed");
    TEST_ASSERT(hmap->count == 2, "Count should be 2");

    // Delete empty string key
    bool delete_ok = mvn_hmap_delete_cstr(hmap, "");
    TEST_ASSERT(delete_ok, "Deleting empty string key failed");
    TEST_ASSERT(hmap->count == 1, "Count should be 1 after deleting empty string key");

    // Verify empty string key is gone
    val = mvn_hmap_get_cstr(hmap, "");
    TEST_ASSERT(val == NULL, "Empty string key should be NULL after delete");

    // Verify other key remains
    val = mvn_hmap_get_cstr(hmap, "other");
    TEST_ASSERT(val != NULL && val->type == MVN_VAL_BOOL && val->b == true,
                "Other key retrieval failed after deleting empty string key");

    mvn_hmap_free(hmap);
    return true; // Test passed
}

/**
 * @brief Tests hash map operations when the map pointer itself is NULL.
 */
static bool test_hmap_operations_on_null_map(void)
{
    mvn_str_t *temp_key_str = mvn_str_new("anyKey");
    TEST_ASSERT(temp_key_str != NULL, "Failed to create temp_key_str for NULL map test");
    mvn_val_t temp_value = mvn_val_i32(123);

    // Test mvn_hmap_get with NULL map
    mvn_val_t *got_val = mvn_hmap_get(NULL, temp_key_str);
    TEST_ASSERT(got_val == NULL, "mvn_hmap_get(NULL, key) should return NULL");

    // Test mvn_hmap_set with NULL map
    bool set_flag = mvn_hmap_set(NULL, temp_key_str, temp_value); // temp_key_str won't be consumed
    TEST_ASSERT(!set_flag, "mvn_hmap_set(NULL, key, val) should return false");

    // Test mvn_hmap_delete with NULL map
    bool delete_flag = mvn_hmap_delete(NULL, temp_key_str); // temp_key_str won't be consumed
    TEST_ASSERT(!delete_flag, "mvn_hmap_delete(NULL, key) should return false");

    // Test _cstr variants with NULL map
    got_val = mvn_hmap_get_cstr(NULL, "anyKeyCstr");
    TEST_ASSERT(got_val == NULL, "mvn_hmap_get_cstr(NULL, key_cstr) should return NULL");

    set_flag = mvn_hmap_set_cstr(NULL, "anyKeyCstr", temp_value);
    TEST_ASSERT(!set_flag, "mvn_hmap_set_cstr(NULL, key_cstr, val) should return false");

    delete_flag = mvn_hmap_delete_cstr(NULL, "anyKeyCstr");
    TEST_ASSERT(!delete_flag, "mvn_hmap_delete_cstr(NULL, key_cstr) should return false");

    mvn_str_free(temp_key_str); // Clean up the temporary key
    // temp_value is primitive, no explicit free needed for its content here

    return true; // Test passed
}

/**
 * @brief Tests hash map operations with NULL key parameters.
 */
static bool test_hmap_operations_with_null_key(void)
{
    mvn_hmap_t *hmap_ptr = mvn_hmap_new();
    TEST_ASSERT(hmap_ptr != NULL, "Failed to create hmap_ptr for NULL key test");
    mvn_val_t temp_value = mvn_val_i32(456);

    // Test mvn_hmap_get with NULL key
    mvn_val_t *got_val = mvn_hmap_get(hmap_ptr, NULL);
    TEST_ASSERT(got_val == NULL, "mvn_hmap_get(map, NULL) should return NULL");

    // Test mvn_hmap_set with NULL key (mvn_str_t*)
    // This should fail as a NULL key cannot be processed.
    bool set_flag = mvn_hmap_set(hmap_ptr, NULL, temp_value);
    TEST_ASSERT(!set_flag, "mvn_hmap_set(map, NULL, val) should return false");
    TEST_ASSERT(hmap_ptr->count == 0, "Map count should be 0 after failed set with NULL key");

    // Test mvn_hmap_delete with NULL key (mvn_str_t*)
    bool delete_flag = mvn_hmap_delete(hmap_ptr, NULL);
    TEST_ASSERT(!delete_flag, "mvn_hmap_delete(map, NULL) should return false");
    TEST_ASSERT(hmap_ptr->count == 0, "Map count should be 0 after failed delete with NULL key");

    // Test _cstr variants with NULL key_cstr
    got_val = mvn_hmap_get_cstr(hmap_ptr, NULL);
    TEST_ASSERT(got_val == NULL, "mvn_hmap_get_cstr(map, NULL) should return NULL");

    set_flag = mvn_hmap_set_cstr(hmap_ptr, NULL, temp_value);
    TEST_ASSERT(!set_flag, "mvn_hmap_set_cstr(map, NULL, val) should return false");
    TEST_ASSERT(hmap_ptr->count == 0, "Map count should be 0 after failed set_cstr with NULL key");

    delete_flag = mvn_hmap_delete_cstr(hmap_ptr, NULL);
    TEST_ASSERT(!delete_flag, "mvn_hmap_delete_cstr(map, NULL) should return false");
    TEST_ASSERT(hmap_ptr->count == 0,
                "Map count should be 0 after failed delete_cstr with NULL key");

    mvn_hmap_free(hmap_ptr);
    // temp_value is primitive, no explicit free needed for its content here

    return true; // Test passed
}

/**
 * @brief Tests mvn_hmap_new_capacity with extremely large capacity values that should fail.
 */
static bool test_hmap_new_capacity_overflow(void)
{
    mvn_hmap_t *hmap_ptr = NULL;

    // Test with SIZE_MAX, should definitely cause an overflow.
    // The actual point of failure might be before multiplication if capacity itself is too large
    // for internal calculations, or during the multiplication for allocation size.
    hmap_ptr = mvn_hmap_new_capacity(SIZE_MAX);
    TEST_ASSERT(hmap_ptr == NULL, "mvn_hmap_new_capacity(SIZE_MAX) should return NULL");

    // Test with a capacity that's just over the limit for sizeof(mvn_hmap_entry_t*) or similar
    // internal structures. This is harder to pinpoint without knowing internal details, but
    // SIZE_MAX / sizeof(void*) might be a proxy.
    if (sizeof(void *) > 0) { // Avoid division by zero
        size_t near_overflow_capacity = (SIZE_MAX / sizeof(void *)) + 1;
        if (near_overflow_capacity > 0) { // Check if it wrapped
            hmap_ptr = mvn_hmap_new_capacity(near_overflow_capacity);
            TEST_ASSERT(hmap_ptr == NULL,
                        "mvn_hmap_new_capacity with near_overflow_capacity should return NULL");
        }
    }
    return true; // Test passed
}

/**
 * @brief Tests setting elements into a hash map initially created with zero capacity.
 */
static bool test_hmap_set_into_zero_capacity_map(void)
{
    mvn_hmap_t *zero_cap_hmap = mvn_hmap_new_capacity(0);
    TEST_ASSERT(zero_cap_hmap != NULL, "Failed to create zero-capacity hash map");
    TEST_ASSERT(zero_cap_hmap->count == 0, "Initial count should be 0");
    TEST_ASSERT(zero_cap_hmap->capacity == 0, "Initial capacity should be 0");
    TEST_ASSERT(zero_cap_hmap->buckets == NULL, "Initial buckets should be NULL");

    // First set should trigger allocation and resize
    bool set_ok = mvn_hmap_set_cstr(zero_cap_hmap, "key1", mvn_val_i32(100));
    TEST_ASSERT(set_ok, "Set into zero-capacity hash map failed");
    TEST_ASSERT(zero_cap_hmap->count == 1, "Count should be 1 after first set");
    TEST_ASSERT(zero_cap_hmap->capacity > 0, "Capacity should be > 0 after first set");
    TEST_ASSERT(zero_cap_hmap->buckets != NULL, "Buckets should not be NULL after first set");

    mvn_val_t *val_ptr = mvn_hmap_get_cstr(zero_cap_hmap, "key1");
    TEST_ASSERT(val_ptr != NULL && val_ptr->type == MVN_VAL_I32 && val_ptr->i32 == 100,
                "Value verification failed after set into zero-cap map");

    // Add another to ensure it's stable
    set_ok = mvn_hmap_set_cstr(zero_cap_hmap, "key2", mvn_val_str("value_two"));
    TEST_ASSERT(set_ok, "Second set into zero-capacity hash map failed");
    TEST_ASSERT(zero_cap_hmap->count == 2, "Count should be 2 after second set");

    mvn_hmap_free(zero_cap_hmap);
    return true; // Test passed
}

/**
 * @brief Tests mvn_hmap_set with an mvn_str_t* key representing an empty string.
 */
static bool test_hmap_set_empty_mvn_str_key(void)
{
    mvn_hmap_t *hmap_ptr = mvn_hmap_new();
    TEST_ASSERT(hmap_ptr != NULL, "Failed to create hash map for empty mvn_str_t key test");

    mvn_str_t *empty_key_str = mvn_str_new(""); // Create an empty mvn_str_t
    TEST_ASSERT(empty_key_str != NULL, "Failed to create empty mvn_str_t key");
    TEST_ASSERT(empty_key_str->length == 0, "Empty mvn_str_t key length should be 0");

    // Set with empty mvn_str_t key (mvn_hmap_set takes ownership)
    bool set_ok = mvn_hmap_set(hmap_ptr, empty_key_str, mvn_val_i32(789));
    TEST_ASSERT(set_ok, "Setting with empty mvn_str_t key failed");
    TEST_ASSERT(hmap_ptr->count == 1, "Count should be 1 after setting empty mvn_str_t key");
    // empty_key_str is now owned by hmap_ptr

    // Get with another empty mvn_str_t key for lookup
    mvn_str_t *lookup_empty_key_str = mvn_str_new("");
    TEST_ASSERT(lookup_empty_key_str != NULL, "Failed to create lookup empty mvn_str_t");
    mvn_val_t *val_ptr = mvn_hmap_get(hmap_ptr, lookup_empty_key_str);
    TEST_ASSERT(val_ptr != NULL && val_ptr->type == MVN_VAL_I32 && val_ptr->i32 == 789,
                "Getting with empty mvn_str_t key failed or value mismatch");
    mvn_str_free(lookup_empty_key_str); // Free the lookup key

    mvn_hmap_free(hmap_ptr); // Frees the map and the original empty_key_str
    return true;             // Test passed
}

/**
 * @brief Tests mvn_hmap_set_cstr with a C-string key containing an embedded null character.
 */
static bool test_hmap_key_cstr_with_embedded_null(void)
{
    mvn_hmap_t *hmap_ptr = mvn_hmap_new();
    TEST_ASSERT(hmap_ptr != NULL, "Failed to create hmap for embedded null cstr key test");

    const char *key_with_null_cstr = "abc\0def"; // Effective key should be "abc"
    bool        set_ok = mvn_hmap_set_cstr(hmap_ptr, key_with_null_cstr, mvn_val_i32(111));
    TEST_ASSERT(set_ok, "Setting with cstr key with embedded null failed");
    TEST_ASSERT(hmap_ptr->count == 1, "Count should be 1");

    // Try to get with "abc"
    mvn_val_t *val_ptr = mvn_hmap_get_cstr(hmap_ptr, "abc");
    TEST_ASSERT(val_ptr != NULL && val_ptr->type == MVN_VAL_I32 && val_ptr->i32 == 111,
                "Getting with truncated key 'abc' failed");

    // Try to get with the original pointer (should still effectively be "abc")
    val_ptr = mvn_hmap_get_cstr(hmap_ptr, key_with_null_cstr);
    TEST_ASSERT(val_ptr != NULL && val_ptr->type == MVN_VAL_I32 && val_ptr->i32 == 111,
                "Getting with original cstr key with embedded null failed");

    // Try to get with "abc\0def" (if lookup also truncates, this will find "abc")
    val_ptr = mvn_hmap_get_cstr(hmap_ptr, "abc\0def");
    TEST_ASSERT(val_ptr != NULL && val_ptr->type == MVN_VAL_I32 && val_ptr->i32 == 111,
                "Getting with explicit 'abc\\0def' cstr key failed");

    // Try to get with "abcdef" (should not be found)
    val_ptr = mvn_hmap_get_cstr(hmap_ptr, "abcdef");
    TEST_ASSERT(val_ptr == NULL, "Getting with 'abcdef' should not find the key");

    mvn_hmap_free(hmap_ptr);
    return true; // Test passed
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
    RUN_TEST(test_hmap_mvn_str_keys);
    RUN_TEST(test_hmap_free_null);
    RUN_TEST(test_hmap_empty_string_key);
    RUN_TEST(test_hmap_operations_on_null_map);
    RUN_TEST(test_hmap_operations_with_null_key);
    RUN_TEST(test_hmap_new_capacity_overflow);       // Added
    RUN_TEST(test_hmap_set_into_zero_capacity_map);  // Added
    RUN_TEST(test_hmap_set_empty_mvn_str_key);       // Added
    RUN_TEST(test_hmap_key_cstr_with_embedded_null); // Added

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
