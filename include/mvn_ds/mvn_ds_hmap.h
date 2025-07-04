/*
 * Copyright (c) 2024 Jake Larson
 */
#ifndef MVN_DS_HMAP_H
#define MVN_DS_HMAP_H

#include "mvn_ds_types.h" // Include the structure definitions

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// Default initial capacity for new hash maps created with mvn_hmap_new()
#define MVN_DS_HMAP_INITIAL_CAPACITY 8
// Factor by which the hash map capacity grows when resizing
#define MVN_DS_HMAP_GROWTH_FACTOR 2
// Load factor threshold that triggers a resize
#define MVN_DS_HMAP_LOAD_FACTOR 0.75

// --- Hash Map Operations ---

// Creates a new, empty hash map with a default initial capacity.
mvn_hmap_t *mvn_hmap_new(void);

// Creates a new, empty hash map with a specific initial capacity.
mvn_hmap_t *mvn_hmap_new_capacity(size_t capacity);

// Frees the memory associated with a hash map, including all keys and values.
void mvn_hmap_free(mvn_hmap_t *hmap);

// Sets a key-value pair in the hash map using an owned mvn_str_t key.
bool mvn_hmap_set(mvn_hmap_t *hmap, mvn_str_t *key, mvn_val_t value);

// Sets a key-value pair using a C string for the key.
bool mvn_hmap_set_cstr(mvn_hmap_t *hmap, const char *key_cstr, mvn_val_t value);

// Retrieves a pointer to the value associated with a given mvn_str_t key.
mvn_val_t *mvn_hmap_get(const mvn_hmap_t *hmap, const mvn_str_t *key);

// Retrieves a pointer to the value associated with a given C string key.
mvn_val_t *mvn_hmap_cstr(const mvn_hmap_t *hmap, const char *key_cstr);

// Deletes a key-value pair from the hash map using an mvn_str_t key.
bool mvn_hmap_delete(mvn_hmap_t *hmap, const mvn_str_t *key);

// Deletes a key-value pair using a C string key.
bool mvn_hmap_delete_cstr(mvn_hmap_t *hmap, const char *key_cstr);

// Returns the number of key-value pairs in the hash map.
size_t mvn_hmap_count(const mvn_hmap_t *hmap);

// Returns the current capacity (number of buckets) of the hash map.
size_t mvn_hmap_capacity(const mvn_hmap_t *hmap);

// Checks if the hash map is empty.
bool mvn_hmap_is_empty(const mvn_hmap_t *hmap);

// Checks if the hash map contains the given mvn_str_t key.
bool mvn_hmap_contains_key(const mvn_hmap_t *hmap, const mvn_str_t *key);

// Checks if the hash map contains the given C string key.
bool mvn_hmap_contains_key_cstr(const mvn_hmap_t *hmap, const char *key_cstr);

// Removes all key-value pairs from the hash map.
// Keys and values are freed. The map's capacity is unchanged.
void mvn_hmap_clear(mvn_hmap_t *hmap);

// Retrieves all keys from the hash map as a new array of strings.
// The caller owns the returned mvn_arr_t and its contents (copies of keys).
// Returns NULL on allocation failure or if hmap is NULL.
mvn_arr_t *mvn_hmap_keys(const mvn_hmap_t *hmap);

// Retrieves all values from the hash map as a new array.
// The caller owns the returned mvn_arr_t and its contents (deep copies of values).
// Returns NULL on allocation failure or if hmap is NULL.
mvn_arr_t *mvn_hmap_values(const mvn_hmap_t *hmap);

// Returns the number of key-value pairs in the hash map (alias for mvn_hmap_count).
size_t mvn_hmap_size(const mvn_hmap_t *hmap);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MVN_DS_HMAP_H */
