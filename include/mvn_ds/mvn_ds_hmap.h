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

// Default initial capacity and growth factor if not defined elsewhere
#define MVN_DS_HMAP_INITIAL_CAPACITY 8
#define MVN_DS_HMAP_GROWTH_FACTOR    2
// Hash map specific load factor
#define MVN_DS_HMAP_LOAD_FACTOR 0.75

// --- Hash Map Operations ---

/**
 * @brief Creates a new, empty hash map with a default initial capacity (0).
 * @return A pointer to the new mvn_hmap_t, or NULL on allocation failure.
 */
mvn_hmap_t *mvn_hmap_new(void);

/**
 * @brief Creates a new, empty hash map with a specific initial capacity.
 * @param capacity The initial number of buckets. If 0, allocation happens on first insert.
 * @return A pointer to the new mvn_hmap_t, or NULL on allocation failure.
 */
mvn_hmap_t *mvn_hmap_new_with_capacity(size_t capacity);

/**
 * @brief Frees the memory associated with a hash map, including all keys and values.
 * @param hmap The hash map to free. Does nothing if NULL.
 */
void mvn_hmap_free(mvn_hmap_t *hmap);

/**
 * @brief Sets a key-value pair in the hash map.
 * Takes ownership of the key string and the value's dynamic data.
 * Frees the existing value if the key already exists. Frees the *provided* key
 * if the key already exists (as the existing key is kept). Resizes if load factor exceeds limit.
 * @param hmap The hash map. Must not be NULL.
 * @param key The key (ownership is taken). Must not be NULL.
 * @param value The value (ownership is taken if dynamic).
 * @return true if successful, false on allocation failure or invalid input.
 */
bool mvn_hmap_set(mvn_hmap_t *hmap, mvn_string_t *key, mvn_val_t value);

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
bool mvn_hmap_set_cstr(mvn_hmap_t *hmap, const char *key_cstr, mvn_val_t value);

/**
 * @brief Retrieves a pointer to the value associated with a given key.
 * Does not transfer ownership. Returns NULL if the key is not found.
 * @param hmap The hash map to search.
 * @param key The key to look up.
 * @return A pointer to the mvn_val_t associated with the key, or NULL if not found.
 */
mvn_val_t *mvn_hmap_get(const mvn_hmap_t *hmap, const mvn_string_t *key);

/**
 * @brief Retrieves a pointer to the value associated with a given C string key.
 * Does not transfer ownership. Returns NULL if the key is not found.
 * @param hmap The hash map to search.
 * @param key_cstr The C string key to look up.
 * @return A pointer to the mvn_val_t associated with the key, or NULL if not found.
 */
mvn_val_t *mvn_hmap_get_cstr(const mvn_hmap_t *hmap, const char *key_cstr);

/**
 * @brief Deletes a key-value pair from the hash map.
 * Frees the key string and the associated value.
 * @param hmap The hash map.
 * @param key The key to delete.
 * @return true if the key was found and deleted, false otherwise.
 */
bool mvn_hmap_delete(mvn_hmap_t *hmap, const mvn_string_t *key);

/**
 * @brief Deletes a key-value pair using a C string key.
 * Frees the key string and the associated value.
 * @param hmap The hash map.
 * @param key_cstr The C string key to delete.
 * @return true if the key was found and deleted, false otherwise.
 */
bool mvn_hmap_delete_cstr(mvn_hmap_t *hmap, const char *key_cstr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MVN_DS_HMAP_H */
