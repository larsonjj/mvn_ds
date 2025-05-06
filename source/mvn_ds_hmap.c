/*
 * Copyright (c) 2024 Jake Larson
 */
#include "mvn_ds/mvn_ds_hmap.h"

#include "mvn_ds/mvn_ds.h" // For mvn_val_free
#include "mvn_ds/mvn_ds_string.h" // Needed for mvn_string_hash, mvn_string_equal, mvn_string_new, mvn_string_free
#include "mvn_ds/mvn_ds_utils.h" // For memory macros (MVN_DS_*)

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h> // For SIZE_MAX
#include <string.h> // For strlen

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
static mvn_hmap_entry_t *mvn_hmap_find_entry(mvn_hmap_entry_t   *head,
                                             const mvn_string_t *key,
                                             uint32_t            hash,
                                             mvn_hmap_entry_t  **prev)
{
    mvn_hmap_entry_t *current_entry = head;
    if (prev) {
        *prev = NULL;
    }
    while (current_entry != NULL) {
        // Optimization: Check hash first, then full key equality
        if (current_entry->key != NULL && mvn_string_hash(current_entry->key) == hash &&
            mvn_string_equal(current_entry->key, key)) {
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
                uint32_t hash_value = mvn_string_hash(current_entry->key);
                size_t   index_new  = hash_value % new_capacity;

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
mvn_hmap_t *mvn_hmap_new_with_capacity(size_t capacity)
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
    return mvn_hmap_new_with_capacity(MVN_DS_HMAP_INITIAL_CAPACITY);
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
                mvn_string_free(current_entry->key);                // Free the key string
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
 * @brief Sets a key-value pair in the hash map using an owned mvn_string_t key.
 * Takes ownership of the key string and the value's dynamic data.
 * Frees the existing value if the key already exists. Frees the *provided* key
 * if the key already exists (as the existing key is kept). Resizes if load factor exceeds limit.
 * @param hmap The hash map. Must not be NULL.
 * @param key The key (ownership is taken). Must not be NULL.
 * @param value The value (ownership is taken if dynamic).
 * @return true if successful, false on allocation failure or invalid input.
 */
bool mvn_hmap_set(mvn_hmap_t *hmap, mvn_string_t *key, mvn_val_t value)
{
    if (hmap == NULL) {
        mvn_val_free(&value); // Free the value as it won't be used or stored
        return false;         // Key ownership remains with the caller
    }

    if (key == NULL) {
        // Key is NULL, operation cannot proceed.
        mvn_val_free(&value); // Free the value as it won't be used or stored
        // If key is NULL, mvn_string_free(key) would be a no-op.
        return false;
    }

    // Ensure initial capacity if needed (capacity 0)
    if (hmap->capacity == 0) {
        if (!mvn_hmap_adjust_capacity(hmap, MVN_DS_HMAP_INITIAL_CAPACITY)) {
            mvn_string_free(key);
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
            mvn_string_free(key);
            mvn_val_free(&value);
            return false;
        }
        if (!mvn_hmap_adjust_capacity(hmap, new_capacity)) {
            // Resize failed, cannot insert. Free key and value.
            mvn_string_free(key);
            mvn_val_free(&value);
            return false;
        }
    }

    // Calculate hash and index
    uint32_t hash_value = mvn_string_hash(key);
    size_t   index      = hash_value % hmap->capacity;

    // Check if key already exists in the bucket chain
    mvn_hmap_entry_t *entry = mvn_hmap_find_entry(hmap->buckets[index], key, hash_value, NULL);

    if (entry != NULL) {
        // Key exists, replace value. Free old value and the provided key.
        mvn_val_free(&entry->value);
        entry->value = value; // Transfer ownership of new value
        mvn_string_free(key); // Free the provided key, as we keep the existing one
        return true;
    } else {
        // Key doesn't exist, create new entry.
        mvn_hmap_entry_t *new_entry = (mvn_hmap_entry_t *)MVN_DS_MALLOC(sizeof(mvn_hmap_entry_t));
        if (new_entry == NULL) {
            // Allocation failed. Free key and value.
            mvn_string_free(key);
            mvn_val_free(&value);
            return false;
        }
        new_entry->key   = key;   // Take ownership of the provided key
        new_entry->value = value; // Take ownership of the provided value

        // Insert at the head of the bucket's list
        new_entry->next      = hmap->buckets[index];
        hmap->buckets[index] = new_entry;
        hmap->count++;
        return true;
    }
}

/**
 * @brief Sets a key-value pair using a C string for the key.
 * Creates a new mvn_string_t for the key internally and takes ownership.
 * Takes ownership of the value's dynamic data.
 * Frees the existing value if the key already exists. Resizes if load factor exceeds limit.
 * @param hmap The hash map. Must not be NULL.
 * @param key_cstr The C string key. Must not be NULL.
 * @param value The value (ownership is taken if dynamic).
 * @return true if successful, false on allocation failure or invalid input.
 */
bool mvn_hmap_set_cstr(mvn_hmap_t *hmap, const char *key_cstr, mvn_val_t value)
{
    if (key_cstr == NULL) {
        mvn_val_free(&value); // Free value if key is invalid
        return false;
    }
    // Create an owned mvn_string_t from the C string
    mvn_string_t *key = mvn_string_new(key_cstr);
    if (key == NULL) {
        mvn_val_free(&value); // Free value if key allocation fails
        return false;         // Failed to create key string
    }
    // Call the primary set function, which will take ownership of the created key
    return mvn_hmap_set(hmap, key, value);
}

/**
 * @brief Retrieves a pointer to the value associated with a given mvn_string_t key.
 * Does not transfer ownership. Returns NULL if the key is not found or map/key is NULL.
 * @param hmap The hash map to search.
 * @param key The key to look up.
 * @return A pointer to the mvn_val_t associated with the key, or NULL if not found.
 */
mvn_val_t *mvn_hmap_get(const mvn_hmap_t *hmap, const mvn_string_t *key)
{
    if (hmap == NULL || key == NULL || hmap->capacity == 0 || hmap->buckets == NULL) {
        return NULL;
    }

    uint32_t hash_value = mvn_string_hash(key);
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
mvn_val_t *mvn_hmap_get_cstr(const mvn_hmap_t *hmap, const char *key_cstr)
{
    if (hmap == NULL || key_cstr == NULL || hmap->capacity == 0 || hmap->buckets == NULL) {
        return NULL;
    }

    // Create a temporary heap-allocated key for lookup
    mvn_string_t *temp_key_ptr = mvn_string_new(key_cstr);
    if (temp_key_ptr == NULL) {
        fprintf(stderr, "[MVN_DS_HMAP] Failed to allocate temporary key for get_cstr.\n");
        return NULL; // Allocation failure
    }

    uint32_t hash_value = mvn_string_hash(temp_key_ptr);
    size_t   index      = hash_value % hmap->capacity;

    mvn_hmap_entry_t *entry =
        mvn_hmap_find_entry(hmap->buckets[index], temp_key_ptr, hash_value, NULL);

    mvn_string_free(temp_key_ptr); // Free the temporary key

    return (entry != NULL) ? &entry->value : NULL;
}

/**
 * @brief Deletes a key-value pair from the hash map using an mvn_string_t key.
 * Frees the key string and the associated value stored in the map.
 * @param hmap The hash map. Must not be NULL.
 * @param key The key to delete. Must not be NULL.
 * @return true if the key was found and deleted, false otherwise.
 */
bool mvn_hmap_delete(mvn_hmap_t *hmap, const mvn_string_t *key)
{
    if (hmap == NULL || key == NULL || hmap->capacity == 0 || hmap->buckets == NULL) {
        return false;
    }

    uint32_t hash_value = mvn_string_hash(key);
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
    mvn_string_free(entry->key);
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
    mvn_string_t *temp_key_ptr = mvn_string_new(key_cstr);
    if (temp_key_ptr == NULL) {
        fprintf(stderr, "[MVN_DS_HMAP] Failed to allocate temporary key for delete_cstr.\n");
        return false; // Allocation failure
    }

    // Call the primary delete function using the temporary key
    bool deleted = mvn_hmap_delete(hmap, temp_key_ptr);

    mvn_string_free(temp_key_ptr); // Free the temporary key

    return deleted;
}
