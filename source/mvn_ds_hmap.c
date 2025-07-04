/*
 * Copyright (c) 2024 Jake Larson
 */
#include "mvn_ds/mvn_ds_hmap.h"

#include "mvn_ds/mvn_ds.h"       // For mvn_val_free, mvn_val_deep_copy, mvn_val_str_take
#include "mvn_ds/mvn_ds_arr.h"   // For mvn_arr_new_capacity, mvn_arr_push
#include "mvn_ds/mvn_ds_str.h"   // For mvn_str_free, mvn_str_new, mvn_str_hash
#include "mvn_ds/mvn_ds_utils.h" // For MVN_DS_MALLOC, MVN_DS_FREE, MVN_DS_CALLOC

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h> // For SIZE_MAX
#include <string.h> // For strcmp, strlen

// --- Static Helper Functions ---

/**
 * @internal
 * @brief Finds an entry in a hash map bucket chain.
 * @param head Head of the bucket's linked list.
 * @param key The key to search for.
 * @param hash The precomputed hash of the key.
 * @param[out] prev Optional pointer to store the previous entry (for deletion).
 * @return Pointer to the found entry, or NULL if not found.
 */
static mvn_hmap_entry_t *mvn_hmap_find_entry(mvn_hmap_entry_t *head,
                                             const mvn_str_t  *key,
                                             uint32_t hash, // Hash of the key being searched for
                                             mvn_hmap_entry_t **prev)
{
    mvn_hmap_entry_t *current_entry = head;
    if (prev) {
        *prev = NULL;
    }
    while (current_entry != NULL) {
        // Optimization: Check hash first, then full key equality
        if (current_entry->hash == hash && // Compare stored hash with the search key's hash
            current_entry->key != NULL &&  // Key should not be NULL here
            mvn_str_equal(current_entry->key, key)) {
            return current_entry;
        }
        if (prev) {
            *prev = current_entry;
        }
        current_entry = current_entry->next;
    }
    return NULL;
}

/**
 * @internal
 * @brief Resizes the hash map's bucket array and rehashes entries.
 * @param hmap The hash map to resize.
 * @param new_capacity The desired new capacity (must be > 0).
 * @return true if successful, false on allocation failure.
 */
static bool mvn_hmap_adjust_capacity(mvn_hmap_t *hmap, size_t new_capacity)
{
    assert(hmap != NULL);
    assert(new_capacity > 0); // Should not be called with 0

    // Check for overflow before calculating allocation size
    if (new_capacity > SIZE_MAX / sizeof(mvn_hmap_entry_t *)) {
        fprintf(stderr, "[MVN_DS_HMAP] Hash map resize capacity overflow.\n");
        return false;
    }
    size_t allocation_size = new_capacity * sizeof(mvn_hmap_entry_t *);

    // Allocate new bucket array, initialized to NULL
    mvn_hmap_entry_t **new_buckets = (mvn_hmap_entry_t **)MVN_DS_CALLOC(1, allocation_size);
    if (new_buckets == NULL) {
        // Allocation failed, keep the old table. This is problematic.
        fprintf(stderr, "[MVN_DS_HMAP] Hash map resize failed - out of memory.\n");
        return false; // Indicate failure
    }

    // Rehash all existing entries into the new buckets
    for (size_t index_old = 0; index_old < hmap->capacity; index_old++) {
        mvn_hmap_entry_t *current_entry = hmap->buckets != NULL ? hmap->buckets[index_old] : NULL;
        while (current_entry != NULL) {
            // Store next entry before modifying current
            mvn_hmap_entry_t *next_entry = current_entry->next;

            // Recalculate index in the new bucket array
            // Ensure entry->key is valid before hashing
            if (current_entry->key != NULL) {
                size_t index_new = current_entry->hash % new_capacity; // Use stored hash

                // Insert entry at the head of the new bucket's list
                current_entry->next    = new_buckets[index_new];
                new_buckets[index_new] = current_entry;
            } else {
                // Handle case where key is somehow NULL (shouldn't happen if set correctly)
                // Free the entry to avoid leaks, though this indicates a prior issue.
                fprintf(stderr,
                        "[MVN_DS_HMAP] Warning: Found entry with NULL key during resize.\n");
                mvn_val_free(&current_entry->value);
                MVN_DS_FREE(current_entry);
            }

            // Move to the next entry in the old bucket
            current_entry = next_entry;
        }
    }

    // Free the old bucket array
    MVN_DS_FREE(hmap->buckets);

    // Update map with new buckets and capacity
    hmap->buckets  = new_buckets;
    hmap->capacity = new_capacity;

    return true;
}

// --- Hash Map Implementation ---

/**
 * @brief Creates a new, empty hash map with a specific initial capacity.
 * @param capacity The initial number of buckets. If 0, allocation happens on first insert.
 * @return A pointer to the new mvn_hmap_t, or NULL on allocation failure.
 */
mvn_hmap_t *mvn_hmap_new_capacity(size_t capacity)
{
    mvn_hmap_t *hmap_ptr = (mvn_hmap_t *)MVN_DS_MALLOC(sizeof(mvn_hmap_t));
    if (hmap_ptr == NULL) {
        return NULL;
    }

    hmap_ptr->count = 0;
    // Use provided capacity directly. If 0, buckets will be NULL.
    hmap_ptr->capacity = capacity;

    if (hmap_ptr->capacity > 0) {
        // Use calloc to initialize bucket pointers to NULL
        // Check for overflow before calculating allocation size
        if (hmap_ptr->capacity > SIZE_MAX / sizeof(mvn_hmap_entry_t *)) {
            MVN_DS_FREE(hmap_ptr);
            fprintf(stderr, "[MVN_DS_HMAP] Hash map initial capacity overflow.\n");
            return NULL;
        }
        size_t allocation_size = hmap_ptr->capacity * sizeof(mvn_hmap_entry_t *);
        hmap_ptr->buckets      = (mvn_hmap_entry_t **)MVN_DS_CALLOC(1, allocation_size);
        if (hmap_ptr->buckets == NULL) {
            MVN_DS_FREE(hmap_ptr);
            return NULL;
        }
    } else {
        hmap_ptr->buckets = NULL;
    }
    return hmap_ptr;
}

/**
 * @brief Creates a new, empty hash map with a default initial capacity.
 * Uses MVN_DS_HMAP_INITIAL_CAPACITY defined in the header.
 * @return A pointer to the new mvn_hmap_t, or NULL on allocation failure.
 */
mvn_hmap_t *mvn_hmap_new(void)
{
    // Use MVN_DS_HMAP_INITIAL_CAPACITY by default
    return mvn_hmap_new_capacity(MVN_DS_HMAP_INITIAL_CAPACITY);
}

/**
 * @brief Frees the memory associated with a hash map, including all keys and values.
 * Iterates through all buckets and entries, freeing each key string, value, and entry struct.
 * Finally frees the bucket array and the map struct itself.
 * @param hmap The hash map to free. Does nothing if NULL.
 */
void mvn_hmap_free(mvn_hmap_t *hmap)
{
    if (hmap == NULL) {
        return;
    }
    // Check if buckets array exists before accessing
    if (hmap->buckets != NULL) {
        for (size_t index = 0; index < hmap->capacity; index++) {
            mvn_hmap_entry_t *current_entry = hmap->buckets[index];
            while (current_entry != NULL) {
                mvn_hmap_entry_t *next_entry = current_entry->next; // Store next pointer
                mvn_str_free(current_entry->key);                   // Free the key string
                mvn_val_free(&current_entry->value);                // Free the value (recursively)
                MVN_DS_FREE(current_entry);                         // Free the entry struct
                current_entry = next_entry;                         // Move to the next entry
            }
        }
        MVN_DS_FREE(hmap->buckets); // Free the bucket array
    }
    MVN_DS_FREE(hmap); // Free the map struct
}

/**
 * @brief Sets a key-value pair in the hash map using an owned mvn_str_t key.
 * Takes ownership of the key string and the value's dynamic data.
 * Frees the existing value if the key already exists. Frees the *provided* key
 * if the key already exists (as the existing key is kept). Resizes if load factor exceeds limit.
 * @param hmap The hash map. Must not be NULL.
 * @param key The key (ownership is taken). Must not be NULL.
 * @param value The value (ownership is taken if dynamic).
 * @return true if successful, false on allocation failure or invalid input.
 */
bool mvn_hmap_set(mvn_hmap_t *hmap, mvn_str_t *key, mvn_val_t value)
{
    if (hmap == NULL) {
        mvn_val_free(&value); // Free the value as it won't be used or stored
        return false;         // Key ownership remains with the caller
    }

    if (key == NULL) {
        // Key is NULL, operation cannot proceed.
        mvn_val_free(&value); // Free the value as it won't be used or stored
        // If key is NULL, mvn_str_free(key) would be a no-op.
        return false;
    }

    // Ensure initial capacity if needed (capacity 0)
    if (hmap->capacity == 0) {
        if (!mvn_hmap_adjust_capacity(hmap, MVN_DS_HMAP_INITIAL_CAPACITY)) {
            mvn_str_free(key);
            mvn_val_free(&value);
            return false; // Failed to allocate initial buckets
        }
    }
    // Resize if load factor *would* exceed threshold after adding the new element
    // Check capacity > 0 before division
    else if ((double)(hmap->count + 1) / hmap->capacity >= MVN_DS_HMAP_LOAD_FACTOR) {
        size_t new_capacity = hmap->capacity * MVN_DS_HMAP_GROWTH_FACTOR;
        // Check for overflow during growth calculation
        if (new_capacity < hmap->capacity) {
            fprintf(stderr,
                    "[MVN_DS_HMAP] Hash map capacity overflow during resize calculation.\n");
            // Attempt to recover if possible, maybe just double? Or fail?
            // For now, fail if overflow detected.
            mvn_str_free(key);
            mvn_val_free(&value);
            return false;
        }
        if (!mvn_hmap_adjust_capacity(hmap, new_capacity)) {
            // Resize failed, cannot insert. Free key and value.
            mvn_str_free(key);
            mvn_val_free(&value);
            return false;
        }
    }

    // Calculate hash and index
    uint32_t hash_value = mvn_str_hash(key);
    size_t   index      = hash_value % hmap->capacity;

    // Check if key already exists in the bucket chain
    mvn_hmap_entry_t *entry = mvn_hmap_find_entry(hmap->buckets[index], key, hash_value, NULL);

    if (entry != NULL) {
        // Key exists, replace value. Free old value and the provided key.
        mvn_val_free(&entry->value);
        entry->value = value; // Transfer ownership of new value
        mvn_str_free(key);    // Free the provided key, as we keep the existing one
        return true;
    } else {
        // Key doesn't exist, create new entry.
        mvn_hmap_entry_t *new_entry = (mvn_hmap_entry_t *)MVN_DS_MALLOC(sizeof(mvn_hmap_entry_t));
        if (new_entry == NULL) {
            // Allocation failed. Free key and value.
            mvn_str_free(key);
            mvn_val_free(&value);
            return false;
        }
        new_entry->key   = key;        // Take ownership of the provided key
        new_entry->hash  = hash_value; // Store the pre-calculated hash
        new_entry->value = value;      // Take ownership of the provided value

        // Insert at the head of the bucket's list
        new_entry->next      = hmap->buckets[index];
        hmap->buckets[index] = new_entry;
        hmap->count++;
        return true;
    }
}

/**
 * @brief Sets a key-value pair using a C string for the key.
 * Creates a new mvn_str_t for the key internally.
 * If the operation is successful and it's a new key, mvn_hmap_set takes ownership of the created
 * key. If the operation is successful and it's a key replacement, mvn_hmap_set frees the created
 * key. If the operation fails (e.g., hmap is NULL or internal allocation failure), this function
 * ensures the created key is freed. Takes ownership of the value's dynamic data if the set is
 * successful. Frees the existing value if the key already exists. Resizes if load factor exceeds
 * limit.
 * @param hmap The hash map.
 * @param key_cstr The C string key. Must not be NULL.
 * @param value The value (ownership is taken if dynamic and set is successful).
 * @return true if successful, false on allocation failure or invalid input.
 */
bool mvn_hmap_set_cstr(mvn_hmap_t *hmap, const char *key_cstr, mvn_val_t value)
{
    if (key_cstr == NULL) {
        mvn_val_free(&value); // Free value if key_cstr is invalid
        return false;
    }
    // Create an owned mvn_str_t from the C string
    mvn_str_t *key_obj = mvn_str_new(key_cstr);
    if (key_obj == NULL) {
        mvn_val_free(&value); // Free value if key allocation fails
        return false;         // Failed to create key string
    }

    // Call the primary set function
    bool success = mvn_hmap_set(hmap, key_obj, value);

    if (!success) {
        // If mvn_hmap_set failed, we need to ensure key_obj is freed if mvn_hmap_set
        // did not take ownership or free it.
        // Specifically, if hmap was NULL, mvn_hmap_set returns false
        // without freeing key_obj.
        // If hmap was valid but mvn_hmap_set failed internally (e.g. alloc error),
        // mvn_hmap_set is responsible for freeing key_obj.
        // If hmap was valid and it was a successful replacement, mvn_hmap_set also frees key_obj.
        if (hmap == NULL) {
            mvn_str_free(key_obj);
        }
    }
    // If success and it was a new key insertion, mvn_hmap_set took ownership of key_obj.
    return success;
}

/**
 * @brief Retrieves a pointer to the value associated with a given mvn_str_t key.
 * Does not transfer ownership. Returns NULL if the key is not found or map/key is NULL.
 * @param hmap The hash map to search.
 * @param key The key to look up.
 * @return A pointer to the mvn_val_t associated with the key, or NULL if not found.
 */
mvn_val_t *mvn_hmap_get(const mvn_hmap_t *hmap, const mvn_str_t *key)
{
    if (hmap == NULL || key == NULL || hmap->capacity == 0 || hmap->buckets == NULL) {
        return NULL;
    }

    uint32_t hash_value = mvn_str_hash(key);
    size_t   index      = hash_value % hmap->capacity;

    mvn_hmap_entry_t *entry = mvn_hmap_find_entry(hmap->buckets[index], key, hash_value, NULL);

    return (entry != NULL) ? &entry->value : NULL;
}

/**
 * @brief Retrieves a pointer to the value associated with a given C string key.
 * Does not transfer ownership. Returns NULL if the key is not found or map/key is NULL.
 * @param hmap The hash map to search.
 * @param key_cstr The C string key to look up.
 * @return A pointer to the mvn_val_t associated with the key, or NULL if not found.
 */
mvn_val_t *mvn_hmap_cstr(const mvn_hmap_t *hmap, const char *key_cstr)
{
    if (hmap == NULL || key_cstr == NULL || hmap->capacity == 0 || hmap->buckets == NULL) {
        return NULL;
    }

    // Create a temporary heap-allocated key for lookup
    mvn_str_t *temp_key_ptr = mvn_str_new(key_cstr);
    if (temp_key_ptr == NULL) {
        fprintf(stderr, "[MVN_DS_HMAP] Failed to allocate temporary key for get_cstr.\n");
        return NULL; // Allocation failure
    }

    uint32_t hash_value = mvn_str_hash(temp_key_ptr);
    size_t   index      = hash_value % hmap->capacity;

    mvn_hmap_entry_t *entry =
        mvn_hmap_find_entry(hmap->buckets[index], temp_key_ptr, hash_value, NULL);

    mvn_str_free(temp_key_ptr); // Free the temporary key

    return (entry != NULL) ? &entry->value : NULL;
}

/**
 * @brief Deletes a key-value pair from the hash map using an mvn_str_t key.
 * Frees the key string and the associated value stored in the map.
 * @param hmap The hash map. Must not be NULL.
 * @param key The key to delete. Must not be NULL.
 * @return true if the key was found and deleted, false otherwise.
 */
bool mvn_hmap_delete(mvn_hmap_t *hmap, const mvn_str_t *key)
{
    if (hmap == NULL || key == NULL || hmap->capacity == 0 || hmap->buckets == NULL) {
        return false;
    }

    uint32_t hash_value = mvn_str_hash(key);
    size_t   index      = hash_value % hmap->capacity;

    mvn_hmap_entry_t *prev_entry = NULL;
    mvn_hmap_entry_t *entry =
        mvn_hmap_find_entry(hmap->buckets[index], key, hash_value, &prev_entry);

    if (entry == NULL) {
        return false; // Key not found
    }

    // Unlink the entry from the list
    if (prev_entry == NULL) {
        // Entry was the head of the list
        hmap->buckets[index] = entry->next;
    } else {
        // Entry was in the middle or end
        prev_entry->next = entry->next;
    }

    // Free the entry's key, value, and the entry struct itself
    mvn_str_free(entry->key);
    mvn_val_free(&entry->value);
    MVN_DS_FREE(entry);

    hmap->count--;
    return true;
}

/**
 * @brief Deletes a key-value pair using a C string key.
 * Frees the key string and the associated value stored in the map.
 * @param hmap The hash map. Must not be NULL.
 * @param key_cstr The C string key to delete. Must not be NULL.
 * @return true if the key was found and deleted, false otherwise.
 */
bool mvn_hmap_delete_cstr(mvn_hmap_t *hmap, const char *key_cstr)
{
    if (hmap == NULL || key_cstr == NULL || hmap->capacity == 0 || hmap->buckets == NULL) {
        return false;
    }

    // Create a temporary heap-allocated key for lookup
    mvn_str_t *temp_key_ptr = mvn_str_new(key_cstr);
    if (temp_key_ptr == NULL) {
        fprintf(stderr, "[MVN_DS_HMAP] Failed to allocate temporary key for delete_cstr.\n");
        return false; // Allocation failure
    }

    // Call the primary delete function using the temporary key
    bool deleted = mvn_hmap_delete(hmap, temp_key_ptr);

    mvn_str_free(temp_key_ptr); // Free the temporary key

    return deleted;
}

/**
 * @brief Returns the number of key-value pairs in the hash map.
 * @param hmap The hash map. Can be NULL.
 * @return The number of key-value pairs, or 0 if the map is NULL.
 */
size_t mvn_hmap_count(const mvn_hmap_t *hmap)
{
    /* Copyright (c) 2024 Jake Larson */
    if (hmap == NULL) {
        return 0;
    }
    return hmap->count;
}

/**
 * @brief Returns the current capacity (number of buckets) of the hash map.
 * @param hmap The hash map. Can be NULL.
 * @return The capacity, or 0 if the map is NULL.
 */
size_t mvn_hmap_capacity(const mvn_hmap_t *hmap)
{
    /* Copyright (c) 2024 Jake Larson */
    if (hmap == NULL) {
        return 0;
    }
    return hmap->capacity;
}

/**
 * @brief Checks if the hash map is empty.
 * @param hmap The hash map. Can be NULL.
 * @return true if the map is empty (or NULL), false otherwise.
 */
bool mvn_hmap_is_empty(const mvn_hmap_t *hmap)
{
    /* Copyright (c) 2024 Jake Larson */
    if (hmap == NULL) {
        return true; // A NULL map is considered empty
    }
    return hmap->count == 0;
}

/**
 * @brief Checks if the hash map contains the given mvn_str_t key.
 * @param hmap The hash map. Can be NULL.
 * @param key The key to check for. Can be NULL.
 * @return true if the key exists, false otherwise or if map/key is NULL.
 */
bool mvn_hmap_contains_key(const mvn_hmap_t *hmap, const mvn_str_t *key)
{
    /* Copyright (c) 2024 Jake Larson */
    if (hmap == NULL || key == NULL) {
        return false;
    }
    return mvn_hmap_get(hmap, key) != NULL;
}

/**
 * @brief Checks if the hash map contains the given C string key.
 * @param hmap The hash map. Can be NULL.
 * @param key_cstr The C string key to check for. Can be NULL.
 * @return true if the key exists, false otherwise or if map/key_cstr is NULL.
 */
bool mvn_hmap_contains_key_cstr(const mvn_hmap_t *hmap, const char *key_cstr)
{
    /* Copyright (c) 2024 Jake Larson */
    if (hmap == NULL || key_cstr == NULL) {
        return false;
    }
    return mvn_hmap_cstr(hmap, key_cstr) != NULL;
}

/**
 * @brief Removes all key-value pairs from the hash map.
 * Keys and values are freed. The map's capacity is unchanged.
 * @param hmap The hash map to clear. Can be NULL.
 */
void mvn_hmap_clear(mvn_hmap_t *hmap)
{
    if (hmap == NULL) {
        return;
    }

    for (size_t i = 0; i < hmap->capacity; ++i) {
        mvn_hmap_entry_t *current_entry = hmap->buckets[i];
        while (current_entry != NULL) {
            mvn_hmap_entry_t *next_entry = current_entry->next;
            mvn_str_free(current_entry->key);
            mvn_val_free(&current_entry->value);
            MVN_DS_FREE(current_entry);
            current_entry = next_entry;
        }
        hmap->buckets[i] = NULL; // Clear the bucket pointer
    }
    hmap->count = 0;
}

/**
 * @brief Retrieves all keys from the hash map as a new array of strings.
 * The caller owns the returned mvn_arr_t and its contents (copies of keys).
 * @param hmap The hash map. Can be NULL.
 * @return A new mvn_arr_t containing copies of all keys, or NULL on failure or if hmap is NULL.
 */
mvn_arr_t *mvn_hmap_keys(const mvn_hmap_t *hmap)
{
    if (hmap == NULL) {
        return NULL;
    }

    mvn_arr_t *keys_array = mvn_arr_new_capacity(hmap->count);
    if (keys_array == NULL) {
        return NULL; // Failed to allocate array
    }

    for (size_t i = 0; i < hmap->capacity; ++i) {
        mvn_hmap_entry_t *current_entry = hmap->buckets[i];
        while (current_entry != NULL) {
            if (current_entry->key != NULL) { // Should always be true for valid entries
                mvn_str_t *key_copy = mvn_str_new(current_entry->key->data);
                if (key_copy == NULL) {
                    mvn_arr_free(keys_array); // Clean up partially filled array
                    return NULL;              // Failed to copy key
                }
                if (!mvn_arr_push(keys_array, mvn_val_str_take(key_copy))) {
                    mvn_str_free(key_copy);   // Free the unpushed key copy
                    mvn_arr_free(keys_array); // Clean up
                    return NULL;              // Failed to push to array
                }
            }
            current_entry = current_entry->next;
        }
    }
    return keys_array;
}

/**
 * @brief Retrieves all values from the hash map as a new array.
 * The caller owns the returned mvn_arr_t and its contents (deep copies of values).
 * @param hmap The hash map. Can be NULL.
 * @return A new mvn_arr_t containing deep copies of all values, or NULL on failure or if hmap is NULL.
 */
mvn_arr_t *mvn_hmap_values(const mvn_hmap_t *hmap)
{
    if (hmap == NULL) {
        return NULL;
    }

    mvn_arr_t *values_array = mvn_arr_new_capacity(hmap->count);
    if (values_array == NULL) {
        return NULL; // Failed to allocate array
    }

    for (size_t i = 0; i < hmap->capacity; ++i) {
        mvn_hmap_entry_t *current_entry = hmap->buckets[i];
        while (current_entry != NULL) {
            mvn_val_t value_copy = mvn_val_deep_copy(&current_entry->value);
            // Check if deep copy failed (e.g., returned MVN_VAL_NULL for a non-NULL original due to alloc failure)
            if (value_copy.type == MVN_VAL_NULL && current_entry->value.type != MVN_VAL_NULL) {
                mvn_arr_free(values_array); // Clean up partially filled array
                return NULL;                // Deep copy failed
            }

            if (!mvn_arr_push(values_array, value_copy)) {
                mvn_val_free(&value_copy);  // Free the unpushed value copy
                mvn_arr_free(values_array); // Clean up
                return NULL;                // Failed to push to array
            }
            current_entry = current_entry->next;
        }
    }
    return values_array;
}

/**
 * @brief Returns the number of key-value pairs in the hash map.
 * This is an alias for mvn_hmap_count.
 * @param hmap The hash map. Can be NULL.
 * @return The number of elements, or 0 if hmap is NULL.
 */
size_t mvn_hmap_size(const mvn_hmap_t *hmap)
{
    return mvn_hmap_count(hmap);
}
