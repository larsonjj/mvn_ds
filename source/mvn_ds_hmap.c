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

// --- Internal Helper Functions ---

/**
 * @internal
 * @brief Finds the map entry for a given key.
 * @param buckets The bucket array.
 * @param capacity The capacity of the bucket array.
 * @param key The key to search for. Must not be NULL.
 * @param hash_value The precomputed hash of the key.
 * @return Pointer to the entry if found, NULL otherwise.
 */
static mvn_hmap_entry_t *mvn_hmap_find_entry(mvn_hmap_entry_t  **buckets,
                                             size_t              capacity,
                                             const mvn_string_t *key,
                                             uint32_t            hash_value)
{
    // Assumes capacity > 0, buckets != NULL, key != NULL (checked by callers)
    size_t            index = hash_value % capacity; // Simple modulo distribution
    mvn_hmap_entry_t *entry = buckets[index];

    // Traverse the linked list (chain) at this bucket index
    while (entry != NULL) {
        // Check hash first (quick check), then full string equality
        // Ensure entry->key is not NULL before hashing/comparing
        if (entry->key != NULL && mvn_string_hash(entry->key) == hash_value &&
            mvn_string_equal(entry->key, key)) {
            return entry; // Found it
        }
        entry = entry->next;
    }
    return NULL; // Not found
}

/**
 * @internal
 * @brief Resizes the hash map's bucket array and rehashes entries.
 * @param hmap The hash map to resize. Must not be NULL.
 * @param new_capacity The desired new capacity. Must be > 0.
 * @return true if resize was successful, false otherwise (e.g., allocation failure).
 */
static bool mvn_hmap_adjust_capacity(mvn_hmap_t *hmap, size_t new_capacity)
{
    assert(hmap != NULL);
    assert(new_capacity > 0); // Should not be called with 0

    // Allocate new bucket array, initialized to NULL
    // Check for overflow before calculating allocation size
    if (new_capacity > SIZE_MAX / sizeof(mvn_hmap_entry_t *)) {
        fprintf(stderr, "[MVN_DS_HMAP] Hash map capacity overflow calculating allocation size.\n");
        return false;
    }
    size_t allocation_size = new_capacity * sizeof(mvn_hmap_entry_t *);
    // Use CALLOC for zero-initialization
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
                // This case should ideally not happen if keys are always valid
                fprintf(stderr, "[MVN_DS_HMAP] Warning: NULL key found during hash map resize.\n");
                // Free the problematic entry to avoid leaks
                mvn_val_free(&current_entry->value);
                MVN_DS_FREE(current_entry);
                hmap->count--; // Decrement count as we lost an entry
            }

            current_entry = next_entry; // Move to the next entry in the old chain
        }
    }

    MVN_DS_FREE(hmap->buckets); // Free the old bucket array
    hmap->buckets  = new_buckets;
    hmap->capacity = new_capacity;
    return true; // Indicate success
}

// --- Hash Map Implementation ---

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

mvn_hmap_t *mvn_hmap_new(void)
{
    // Use MVN_DS_HMAP_INITIAL_CAPACITY by default
    return mvn_hmap_new_with_capacity(MVN_DS_HMAP_INITIAL_CAPACITY);
}

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

bool mvn_hmap_set(mvn_hmap_t *hmap, mvn_string_t *key, mvn_val_t value)
{
    if (hmap == NULL || key == NULL) {
        // Invalid input. Free potentially owned value and provided key.
        mvn_val_free(&value);
        mvn_string_free(key); // Safe even if key is NULL
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
        // Check for potential overflow during growth calculation
        if (new_capacity < hmap->capacity) {
            fprintf(stderr,
                    "[MVN_DS_HMAP] Hash map capacity overflow during resize calculation.\n");
            // Free key/value as we cannot insert
            mvn_string_free(key);
            mvn_val_free(&value);
            return false;
        }
        if (!mvn_hmap_adjust_capacity(hmap, new_capacity)) {
            // Resize failed, free key/value
            mvn_string_free(key);
            mvn_val_free(&value);
            return false;
        }
    }

    uint32_t hash_value = mvn_string_hash(key);
    // Capacity is now > 0 and buckets is not NULL
    size_t             index = hash_value % hmap->capacity;
    mvn_hmap_entry_t **bucket_ptr =
        &hmap->buckets[index]; // Pointer to the slot holding the head pointer

    // Traverse chain to check if key exists
    while (*bucket_ptr != NULL) {
        mvn_hmap_entry_t *current_entry = *bucket_ptr;
        // Check hash first, then full equality
        // Ensure entry->key is not NULL
        if (current_entry->key != NULL && mvn_string_hash(current_entry->key) == hash_value &&
            mvn_string_equal(current_entry->key, key)) {
            // Key found - replace value
            mvn_val_free(&current_entry->value); // Free the old value
            current_entry->value = value;        // Assign the new value (takes ownership)
            mvn_string_free(key); // Free the *provided* key, as we keep the existing one
            return true;
        }
        bucket_ptr = &current_entry->next; // Move to the next pointer in the chain
    }

    // Key not found - create new entry
    mvn_hmap_entry_t *new_entry = (mvn_hmap_entry_t *)MVN_DS_MALLOC(sizeof(mvn_hmap_entry_t));
    if (new_entry == NULL) {
        // Allocation failed - free the key and value we were given
        mvn_string_free(key);
        mvn_val_free(&value);
        return false;
    }

    new_entry->key   = key;   // Takes ownership of the provided key
    new_entry->value = value; // Takes ownership of the provided value
    new_entry->next  = NULL;  // It's the new end of the chain

    // Link the new entry into the bucket (at the end of the chain where bucket_ptr points)
    *bucket_ptr = new_entry;
    hmap->count++;
    return true;
}

bool mvn_hmap_set_cstr(mvn_hmap_t *hmap, const char *key_cstr, mvn_val_t value)
{
    if (key_cstr == NULL) {
        mvn_val_free(&value); // Free value if key is invalid
        return false;
    }
    mvn_string_t *key_string = mvn_string_new(key_cstr);
    if (key_string == NULL) {
        mvn_val_free(&value); // Free value if key string creation fails
        return false;
    }
    // mvn_hmap_set takes ownership of key_string and value
    if (!mvn_hmap_set(hmap, key_string, value)) {
        // If set fails, it should have already freed key_string and value
        return false;
    }
    return true;
}

mvn_val_t *mvn_hmap_get(const mvn_hmap_t *hmap, const mvn_string_t *key)
{
    if (hmap == NULL || key == NULL || hmap->capacity == 0 || hmap->buckets == NULL) {
        return NULL;
    }
    uint32_t hash_value = mvn_string_hash(key);
    // Find entry requires non-const buckets**, but get is const.
    // Casting away const here is generally safe if find_entry doesn't modify buckets.
    mvn_hmap_entry_t *entry =
        mvn_hmap_find_entry(((mvn_hmap_t *)hmap)->buckets, hmap->capacity, key, hash_value);
    return entry != NULL ? &entry->value : NULL;
}

mvn_val_t *mvn_hmap_get_cstr(const mvn_hmap_t *hmap, const char *key_cstr)
{
    if (hmap == NULL || key_cstr == NULL || hmap->capacity == 0 || hmap->buckets == NULL) {
        return NULL;
    }
    // Create a temporary string wrapper for searching, without allocating new
    // memory for the chars. Cast discards const, but hash/equal won't modify.
    mvn_string_t temp_key   = {.length = strlen(key_cstr), .capacity = 0, .data = (char *)key_cstr};
    uint32_t     hash_value = mvn_string_hash(&temp_key);

    // Need to cast away const from hmap temporarily to call non-const find_entry
    mvn_hmap_entry_t *entry =
        mvn_hmap_find_entry(((mvn_hmap_t *)hmap)->buckets, hmap->capacity, &temp_key, hash_value);
    return entry != NULL ? &entry->value : NULL;
}

bool mvn_hmap_delete(mvn_hmap_t *hmap, const mvn_string_t *key)
{
    if (hmap == NULL || key == NULL || hmap->capacity == 0 || hmap->buckets == NULL) {
        return false;
    }

    uint32_t hash_value = mvn_string_hash(key);
    size_t   index      = hash_value % hmap->capacity;
    // Pointer to the link pointing to the current entry
    mvn_hmap_entry_t **bucket_ptr = &hmap->buckets[index];
    mvn_hmap_entry_t  *entry      = *bucket_ptr;

    while (entry != NULL) {
        // Check hash and then equality
        // Ensure entry->key is not NULL
        if (entry->key != NULL && mvn_string_hash(entry->key) == hash_value &&
            mvn_string_equal(entry->key, key)) {
            // Found the entry, unlink it
            *bucket_ptr = entry->next; // Make the previous link point to the next entry

            // Free the deleted entry's contents and the entry itself
            mvn_string_free(entry->key);
            mvn_val_free(&entry->value);
            MVN_DS_FREE(entry);
            hmap->count--;
            return true; // Successfully deleted
        }
        // Move to the next entry's link
        bucket_ptr = &entry->next;
        entry      = *bucket_ptr;
    }

    return false; // Key not found
}

bool mvn_hmap_delete_cstr(mvn_hmap_t *hmap, const char *key_cstr)
{
    if (hmap == NULL || key_cstr == NULL || hmap->capacity == 0 || hmap->buckets == NULL) {
        return false;
    }
    // Use the temporary string trick again for lookup
    // Cast discards const, but delete won't modify.
    mvn_string_t temp_key = {.length = strlen(key_cstr), .capacity = 0, .data = (char *)key_cstr};
    // Call the actual delete function
    return mvn_hmap_delete(hmap, &temp_key);
}
