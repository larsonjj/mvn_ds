/*
 * Copyright (c) 2024 Jake Larson
 */
#ifndef MVN_DS_ARRAY_H
#define MVN_DS_ARRAY_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// --- Forward Declarations ---
// Forward declare mvn_val_t to avoid circular dependency with mvn_ds.h
// The full definition is needed in the .c file.
typedef struct mvn_val_t mvn_val_t;

// --- Dynamic Array ---
/**
 * @brief Structure representing a dynamic array of mvn_val_t values.
 */
typedef struct mvn_array_t {
    size_t     count;    /**< Number of elements currently in the array. */
    size_t     capacity; /**< Allocated capacity of the value buffer. */
    mvn_val_t *data;     /**< Pointer to the buffer holding mvn_val_t elements. */
} mvn_array_t;

// --- Array Operations ---

/**
 * @brief Creates a new, empty dynamic array with a default initial capacity.
 * @return A pointer to the new mvn_array_t, or NULL on allocation failure.
 */
mvn_array_t *mvn_array_new(void);

/**
 * @brief Creates a new, empty dynamic array with a specific initial capacity.
 * @param capacity The initial capacity.
 * @return A pointer to the new mvn_array_t, or NULL on allocation failure.
 */
mvn_array_t *mvn_array_new_with_capacity(size_t capacity);

/**
 * @brief Frees the memory associated with a dynamic array, including all contained values.
 * @param array The array to free. Does nothing if NULL.
 */
void mvn_array_free(mvn_array_t *array);

/**
 * @brief Appends a value to the end of the array.
 * The array takes ownership of the value if it's a dynamic type (STRING, ARRAY, HASHMAP).
 * Resizes the array if necessary.
 * @param array The array to append to. Must not be NULL.
 * @param value The value to append. Ownership is transferred to the array.
 * @return true if successful, false on allocation failure or invalid input.
 */
bool mvn_array_push(mvn_array_t *array, mvn_val_t value);

/**
 * @brief Retrieves a pointer to the value at a specific index.
 * Does not transfer ownership. Returns NULL if the index is out of bounds.
 * @param array The array to access.
 * @param index The index of the element to retrieve.
 * @return A pointer to the mvn_val_t at the index, or NULL if out of bounds.
 */
mvn_val_t *mvn_array_get(const mvn_array_t *array, size_t index);

/**
 * @brief Sets the value at a specific index in the array.
 * Frees the existing value at the index before setting the new one.
 * The array takes ownership of the new value if it's a dynamic type.
 * @param array The array to modify. Must not be NULL.
 * @param index The index to set. Must be less than the array's count.
 * @param value The new value. Ownership is transferred to the array.
 * @return true if successful (index was valid), false otherwise (index out of bounds or invalid
 * input).
 */
bool mvn_array_set(mvn_array_t *array, size_t index, mvn_val_t value);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MVN_DS_ARRAY_H */
