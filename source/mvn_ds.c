/*
 * Copyright (c) 2024 Jake Larson
 */
#include "mvn_ds/mvn_ds.h" // Includes string and array headers now

#include "mvn_ds/mvn_ds_utils.h"

#include <assert.h>   // For basic assertions
#include <inttypes.h> // For PRI macros like PRId64
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h> // For int32_t, int64_t etc.
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Still needed for memcpy, strlen etc. in map

// --- Defines ---
// Keep defines used by HMAP here, or move all to a shared internal header
#define MVN_INITIAL_CAPACITY 8 // Used by map
#define MVN_GROWTH_FACTOR    2 // Used by map
#define MVN_MAP_LOAD_FACTOR  0.75

// --- Hash Map Implementation ---

/**
 * @internal
 * @brief Finds the map entry for a given key.
 * @param buckets The bucket array.
 * @param capacity The capacity of the bucket array.
 * @param key The key to search for.
 * @param hash_value The precomputed hash of the key.
 * @return Pointer to the entry if found, NULL otherwise.
 */
static mvn_hmap_entry_t *mvn_hmap_find_entry(mvn_hmap_entry_t  **buckets,
                                             size_t              capacity,
                                             const mvn_string_t *key,
                                             uint32_t            hash_value)
{
    if (capacity == 0 || !buckets || !key) { // Added checks for NULL buckets/key
        return NULL;
    }
    size_t            index = hash_value % capacity; // Simple modulo distribution
    mvn_hmap_entry_t *entry = buckets[index];

    // Traverse the linked list (chain) at this bucket index
    while (entry != NULL) {
        // Check hash first (quick check), then full string equality
        // Ensure entry->key is not NULL before hashing/comparing
        if (entry->key && mvn_string_hash(entry->key) == hash_value &&
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
 * @param hmap The hash map to resize.
 * @param new_capacity The desired new capacity. Must be > 0.
 */
static bool mvn_hmap_adjust_capacity(mvn_hmap_t *hmap, size_t new_capacity)
{
    assert(hmap != NULL);
    assert(new_capacity > 0); // Should not be called with 0

    // Allocate new bucket array, initialized to NULL
    // Check for overflow before calculating allocation size
    if (new_capacity > SIZE_MAX / sizeof(mvn_hmap_entry_t *)) {
        fprintf(stderr, "[MVN_DS] Hash map capacity overflow calculating allocation size.\n");
        return false;
    }
    size_t             allocation_size = new_capacity * sizeof(mvn_hmap_entry_t *);
    mvn_hmap_entry_t **new_buckets     = (mvn_hmap_entry_t **)MVN_DS_CALLOC(
        1, allocation_size); // Use CALLOC for zero-initialization
    if (!new_buckets) {
        // Allocation failed, keep the old table. This is problematic.
        fprintf(stderr, "[MVN_DS] Hash map resize failed - out of memory.\n");
        return false; // Indicate failure
    }

    // Rehash all existing entries into the new buckets
    for (size_t index_old = 0; index_old < hmap->capacity; index_old++) {
        mvn_hmap_entry_t *entry = hmap->buckets ? hmap->buckets[index_old] : NULL;
        while (entry != NULL) {
            mvn_hmap_entry_t *next = entry->next; // Store next entry before modifying current

            // Recalculate index in the new bucket array
            // Ensure entry->key is valid before hashing
            if (entry->key) {
                uint32_t hash_value = mvn_string_hash(entry->key);
                size_t   index_new  = hash_value % new_capacity;

                // Insert entry at the head of the new bucket's list
                entry->next            = new_buckets[index_new];
                new_buckets[index_new] = entry;
            } else {
                // This case should ideally not happen if keys are always valid
                fprintf(stderr, "[MVN_DS] Warning: NULL key found during hash map resize.\n");
                // Free the problematic entry to avoid leaks?
                mvn_val_free(&entry->value);
                MVN_DS_FREE(entry);
                hmap->count--; // Decrement count as we lost an entry
            }

            entry = next; // Move to the next entry in the old chain
        }
    }

    MVN_DS_FREE(hmap->buckets); // Free the old bucket array
    hmap->buckets  = new_buckets;
    hmap->capacity = new_capacity;
    return true; // Indicate success
}

mvn_hmap_t *mvn_hmap_new_with_capacity(size_t capacity)
{
    mvn_hmap_t *hmap = (mvn_hmap_t *)MVN_DS_MALLOC(sizeof(mvn_hmap_t));
    if (!hmap) {
        return NULL;
    }

    hmap->count = 0;
    // Ensure capacity is at least 1 if requested > 0, or 0 otherwise
    hmap->capacity = (capacity > 0) ? capacity : 0;

    if (hmap->capacity > 0) {
        // Use calloc to initialize bucket pointers to NULL
        // Check for overflow before calculating allocation size
        if (hmap->capacity > SIZE_MAX / sizeof(mvn_hmap_entry_t *)) {
            MVN_DS_FREE(hmap);
            fprintf(stderr, "[MVN_DS] Hash map initial capacity overflow.\n");
            return NULL;
        }
        size_t allocation_size = hmap->capacity * sizeof(mvn_hmap_entry_t *);
        hmap->buckets = (mvn_hmap_entry_t **)MVN_DS_CALLOC(1, allocation_size); // Use CALLOC
        if (!hmap->buckets) {
            MVN_DS_FREE(hmap);
            return NULL;
        }
    } else {
        hmap->buckets = NULL;
    }
    return hmap;
}

mvn_hmap_t *mvn_hmap_new(void)
{
    return mvn_hmap_new_with_capacity(MVN_INITIAL_CAPACITY);
}

void mvn_hmap_free(mvn_hmap_t *hmap)
{
    if (!hmap) {
        return;
    }
    for (size_t index = 0; index < hmap->capacity; index++) {
        // Check if buckets array exists before accessing
        mvn_hmap_entry_t *entry = hmap->buckets ? hmap->buckets[index] : NULL;
        while (entry != NULL) {
            mvn_hmap_entry_t *next = entry->next; // Store next pointer
            mvn_string_free(entry->key);          // Free the key string
            mvn_val_free(&entry->value);          // Free the value (recursively)
            MVN_DS_FREE(entry);                   // Free the entry struct
            entry = next;                         // Move to the next entry
        }
    }
    MVN_DS_FREE(hmap->buckets); // Free the bucket array
    MVN_DS_FREE(hmap);          // Free the map struct
}

bool mvn_hmap_set(mvn_hmap_t *hmap, mvn_string_t *key, mvn_val_t value)
{
    if (!hmap || !key) {
        // Invalid input. Free potentially owned value and provided key.
        mvn_val_free(&value);
        mvn_string_free(key); // Safe even if key is NULL
        return false;
    }

    // Ensure initial capacity if needed
    if (hmap->capacity == 0) {
        if (!mvn_hmap_adjust_capacity(hmap, MVN_INITIAL_CAPACITY)) {
            mvn_string_free(key);
            mvn_val_free(&value);
            return false; // Failed to allocate initial buckets
        }
    }
    // Resize if load factor *would* exceed threshold after adding the new element
    // Check capacity > 0 before division
    else if (hmap->capacity > 0 &&
             (double)(hmap->count + 1) / hmap->capacity >= MVN_MAP_LOAD_FACTOR) {
        size_t new_capacity = hmap->capacity * MVN_GROWTH_FACTOR;
        // Check for potential overflow during growth calculation
        if (new_capacity < hmap->capacity) {
            fprintf(stderr, "[MVN_DS] Hash map capacity overflow during resize calculation.\n");
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

    uint32_t           hash_value = mvn_string_hash(key);
    size_t             index      = hash_value % hmap->capacity; // Capacity is now > 0
    mvn_hmap_entry_t **bucket_ptr =
        &hmap->buckets[index]; // Pointer to the slot holding the head pointer

    // Traverse chain to check if key exists
    while (*bucket_ptr != NULL) {
        mvn_hmap_entry_t *entry = *bucket_ptr;
        // Check hash first, then full equality
        // Ensure entry->key is not NULL
        if (entry->key && mvn_string_hash(entry->key) == hash_value &&
            mvn_string_equal(entry->key, key)) {
            // Key found - replace value
            mvn_val_free(&entry->value); // Free the old value
            entry->value = value;        // Assign the new value (takes ownership)
            mvn_string_free(key);        // Free the *provided* key, as we keep the existing one
            return true;
        }
        bucket_ptr = &entry->next; // Move to the next pointer in the chain
    }

    // Key not found - create new entry
    mvn_hmap_entry_t *new_entry = (mvn_hmap_entry_t *)MVN_DS_MALLOC(sizeof(mvn_hmap_entry_t));
    if (!new_entry) {
        // Allocation failed - free the key and value we were given
        mvn_string_free(key);
        mvn_val_free(&value);
        return false;
    }

    new_entry->key   = key;   // Takes ownership of the provided key
    new_entry->value = value; // Takes ownership of the provided value
    new_entry->next  = NULL;  // It's the new end of the chain

    // Link the new entry into the bucket (at the end of the chain where
    // bucket_ptr points)
    *bucket_ptr = new_entry;
    hmap->count++;
    return true;
}

bool mvn_hmap_set_cstr(mvn_hmap_t *hmap, const char *key_cstr, mvn_val_t value)
{
    if (!key_cstr) {
        mvn_val_free(&value); // Free value if key is invalid
        return false;
    }
    mvn_string_t *key_string = mvn_string_new(key_cstr);
    if (!key_string) {
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
    if (!hmap || !key || hmap->capacity == 0) {
        return NULL;
    }
    uint32_t hash_value = mvn_string_hash(key);
    // Need const cast for buckets access if hmap is const, find_entry takes non-const buckets**
    mvn_hmap_entry_t *entry =
        mvn_hmap_find_entry(((mvn_hmap_t *)hmap)->buckets, hmap->capacity, key, hash_value);
    return entry ? &entry->value : NULL;
}

mvn_val_t *mvn_hmap_get_cstr(const mvn_hmap_t *hmap, const char *key_cstr)
{
    if (!hmap || !key_cstr || hmap->capacity == 0) {
        return NULL;
    }
    // Create a temporary string wrapper for searching, without allocating new
    // memory for the chars.
    // Cast discards const, but hash/equal won't modify.
    mvn_string_t temp_key   = {.length = strlen(key_cstr), .capacity = 0, .data = (char *)key_cstr};
    uint32_t     hash_value = mvn_string_hash(&temp_key);

    // Need to cast away const from hmap temporarily to call non-const find_entry
    mvn_hmap_entry_t *entry =
        mvn_hmap_find_entry(((mvn_hmap_t *)hmap)->buckets, hmap->capacity, &temp_key, hash_value);
    return entry ? &entry->value : NULL;
}

bool mvn_hmap_delete(mvn_hmap_t *hmap, const mvn_string_t *key)
{
    if (!hmap || !key || hmap->capacity == 0 || !hmap->buckets) { // Added !hmap->buckets check
        return false;
    }

    uint32_t           hash_value = mvn_string_hash(key);
    size_t             index      = hash_value % hmap->capacity;
    mvn_hmap_entry_t **bucket_ptr = &hmap->buckets[index]; // Pointer to the link pointing to the
                                                           // current entry
    mvn_hmap_entry_t *entry = *bucket_ptr;

    while (entry != NULL) {
        // Check hash and then equality
        // Ensure entry->key is not NULL
        if (entry->key && mvn_string_hash(entry->key) == hash_value &&
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
    if (!hmap || !key_cstr || hmap->capacity == 0) {
        return false;
    }
    // Use the temporary string trick again for lookup
    // Cast discards const, but delete won't modify.
    mvn_string_t temp_key = {.length = strlen(key_cstr), .capacity = 0, .data = (char *)key_cstr};
    // Call the actual delete function
    return mvn_hmap_delete(hmap, &temp_key);
}

// --- Value Implementation ---

mvn_val_t mvn_val_null(void)
{
    return (mvn_val_t){.type = MVN_VAL_NULL};
}
mvn_val_t mvn_val_bool(bool b)
{
    return (mvn_val_t){.type = MVN_VAL_BOOL, .b = b};
}
mvn_val_t mvn_val_i32(int32_t i32)
{
    return (mvn_val_t){.type = MVN_VAL_I32, .i32 = i32};
}
mvn_val_t mvn_val_i64(int64_t i64)
{
    return (mvn_val_t){.type = MVN_VAL_I64, .i64 = i64};
}
mvn_val_t mvn_val_f32(float f32)
{
    return (mvn_val_t){.type = MVN_VAL_F32, .f32 = f32};
}
mvn_val_t mvn_val_f64(double f64)
{
    return (mvn_val_t){.type = MVN_VAL_F64, .f64 = f64};
}

mvn_val_t mvn_val_string(const char *chars)
{
    mvn_string_t *str = mvn_string_new(chars);
    if (!str) {
        return mvn_val_null(); // Handle allocation failure
    }
    return (mvn_val_t){.type = MVN_VAL_STRING, .str = str};
}

mvn_val_t mvn_val_string_take(mvn_string_t *str)
{
    if (!str) {
        return mvn_val_null();
    }
    return (mvn_val_t){.type = MVN_VAL_STRING, .str = str};
}

mvn_val_t mvn_val_array(void)
{
    mvn_array_t *arr = mvn_array_new();
    if (!arr) {
        return mvn_val_null(); // Handle allocation failure
    }
    return (mvn_val_t){.type = MVN_VAL_ARRAY, .arr = arr};
}

mvn_val_t mvn_val_array_take(mvn_array_t *arr)
{
    if (!arr) {
        return mvn_val_null();
    }
    return (mvn_val_t){.type = MVN_VAL_ARRAY, .arr = arr};
}

mvn_val_t mvn_val_hmap(void)
{
    mvn_hmap_t *hmap = mvn_hmap_new();
    if (!hmap) {
        return mvn_val_null(); // Handle allocation failure
    }
    return (mvn_val_t){.type = MVN_VAL_HASHMAP, .hmap = hmap};
}

mvn_val_t mvn_val_hmap_take(mvn_hmap_t *hmap)
{
    if (!hmap) {
        return mvn_val_null();
    }
    return (mvn_val_t){.type = MVN_VAL_HASHMAP, .hmap = hmap};
}

void mvn_val_free(mvn_val_t *value)
{
    if (!value) {
        return;
    }
    switch (value->type) {
            // Dynamic types that need freeing:
        case MVN_VAL_STRING:
            mvn_string_free(value->str);
            break;
        case MVN_VAL_ARRAY:
            mvn_array_free(value->arr);
            break;
        case MVN_VAL_HASHMAP:
            mvn_hmap_free(value->hmap);
            break;
            // Primitive types and NULL don't own heap resources:
        case MVN_VAL_NULL:
        case MVN_VAL_BOOL:
        case MVN_VAL_I32:
        case MVN_VAL_I64:
        case MVN_VAL_F32:
        case MVN_VAL_F64:
            // No action needed
            break;
            // Default case for safety, although all enum values should be handled
        default:
            fprintf(
                stderr, "[MVN_DS] Warning: mvn_val_free called on unknown type %d\n", value->type);
            break;
    }
    // Reset to NULL to prevent double frees and indicate it's no longer valid
    *value = mvn_val_null();
}

void mvn_val_print(const mvn_val_t *value)
{
    if (!value) {
        printf("NULL_VALUE_PTR");
        return;
    }
    switch (value->type) {
        case MVN_VAL_NULL:
            printf("null");
            break;
        case MVN_VAL_BOOL:
            printf(value->b ? "true" : "false");
            break;
        case MVN_VAL_I32:
            printf("%" PRId32, value->i32); // Use PRI macros for portability
            break;
        case MVN_VAL_I64:
            printf("%" PRId64, value->i64);
            break;
        case MVN_VAL_F32:
            printf("%g", value->f32); // %g is often suitable for floats/doubles
            break;
        case MVN_VAL_F64:
            printf("%g", value->f64);
            break;
        case MVN_VAL_STRING:
            // Check str and str->data for validity
            printf("\"%s\"", (value->str && value->str->data) ? value->str->data : "NULL_STR");
            break;
        case MVN_VAL_ARRAY:
            if (!value->arr) {
                printf("NULL_ARR_PTR");
                break;
            }
            printf("[");
            for (size_t index = 0; index < value->arr->count; index++) {
                // Check data pointer before accessing element
                if (value->arr->data) {
                    mvn_val_print(&value->arr->data[index]);
                } else {
                    printf("INVALID_ARR_DATA"); // Should not happen if count > 0
                }

                if (index < value->arr->count - 1) {
                    printf(", ");
                }
            }
            printf("]");
            break;
        case MVN_VAL_HASHMAP: { // Use braces for scope
            if (!value->hmap) {
                printf("NULL_HMAP_PTR");
                break;
            }
            printf("{");
            bool first = true;
            // Check if buckets exist before iterating
            if (value->hmap->buckets) {
                for (size_t index = 0; index < value->hmap->capacity; index++) {
                    mvn_hmap_entry_t *entry = value->hmap->buckets[index];
                    while (entry) {
                        if (!first) {
                            printf(", ");
                        }
                        first = false;
                        // Assume key is always valid if entry exists, check data
                        printf("\"%s\": ",
                               (entry->key && entry->key->data) ? entry->key->data : "NULL_KEY");
                        mvn_val_print(&entry->value);
                        entry = entry->next;
                    }
                }
            }
            printf("}");
            break;
        } // Close brace for case
        default:
            printf("UNKNOWN_TYPE(%d)", value->type);
            break;
    }
}
