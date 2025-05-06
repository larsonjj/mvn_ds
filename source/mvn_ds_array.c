/*
 * Copyright (c) 2024 Jake Larson
 */
#include "mvn_ds/mvn_ds_array.h"

#include "mvn_ds/mvn_ds.h"       // Provides mvn_val_null, mvn_val_free
#include "mvn_ds/mvn_ds_utils.h" // Provides memory macros (MVN_DS_*)

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h> // For SIZE_MAX

// --- Internal Helper Functions ---

/**
 * @internal
 * @brief Reallocates memory for the array data, handling potential errors.
 * Uses MVN_DS_REALLOC and MVN_DS_FREE.
 * @param pointer Existing pointer (or NULL).
 * @param new_size Desired new size. If 0, frees the pointer.
 * @return Pointer to the reallocated memory, or NULL if new_size is 0 or allocation fails.
 */
static void *mvn_reallocate_array(void *pointer, size_t new_size)
{
    // old_size is not needed for standard realloc
    if (new_size == 0) {
        MVN_DS_FREE(pointer);
        return NULL;
    }
    void *result = MVN_DS_REALLOC(pointer, new_size);
    if (result == NULL && new_size > 0) { // Check if allocation actually failed
        fprintf(stderr, "[MVN_DS_ARRAY] Memory reallocation failed!\n");
    }
    return result;
}

/**
 * @internal
 * @brief Ensures array has enough capacity, reallocating if necessary.
 * @param array The array to check/grow. Must not be NULL.
 * @return true if successful (or no resize needed), false on allocation failure.
 */
static bool mvn_array_ensure_capacity(mvn_array_t *array)
{
    assert(array != NULL);
    if (array->count < array->capacity) {
        return true; // Enough space
    }
    size_t old_capacity = array->capacity;
    size_t new_capacity = old_capacity < MVN_DS_ARRAY_INITIAL_CAPACITY ?
                              MVN_DS_ARRAY_INITIAL_CAPACITY :
                              old_capacity * MVN_DS_ARRAY_GROWTH_FACTOR;

    // Check for potential overflow during growth calculation
    if (new_capacity < old_capacity && old_capacity > 0) { // Check if overflow occurred
        fprintf(stderr, "[MVN_DS_ARRAY] Array capacity overflow during resize calculation.\n");
        // Try setting to count + 1 as a last resort if possible
        if (SIZE_MAX - 1 < array->count) {
            return false; // Cannot even add one more element
        }
        new_capacity = array->count + 1;
        if (new_capacity <= old_capacity) { // Still not enough or wrapped around
            return false;
        }
    } else if (new_capacity == 0 && old_capacity == 0) { // Handle initial allocation case
        new_capacity = MVN_DS_ARRAY_INITIAL_CAPACITY;
    }

    // Check for overflow before calculating allocation size
    if (new_capacity > SIZE_MAX / sizeof(mvn_val_t)) {
        fprintf(stderr, "[MVN_DS_ARRAY] Array capacity overflow calculating allocation size.\n");
        return false;
    }
    size_t allocation_size = new_capacity * sizeof(mvn_val_t);

    // Pass only pointer and new_size to the local mvn_reallocate_array
    mvn_val_t *new_data =
        (mvn_val_t *)mvn_reallocate_array(array->data, allocation_size); // Updated call site
    if (!new_data) {
        return false;
    }

    array->data     = new_data;
    array->capacity = new_capacity;

    // Initialize new slots to NULL to prevent freeing uninitialized memory later
    for (size_t index = old_capacity; index < new_capacity; ++index) {
        array->data[index] = mvn_val_null();
    }
    return true;
}

// --- Array Implementation ---

/**
 * @brief Creates a new, empty dynamic array with a specific initial capacity.
 * @param capacity The initial capacity. If 0, no initial buffer is allocated.
 * @return A pointer to the new mvn_array_t, or NULL on allocation failure.
 */
mvn_array_t *mvn_array_new_with_capacity(size_t capacity)
{
    mvn_array_t *array = (mvn_array_t *)MVN_DS_MALLOC(sizeof(mvn_array_t));
    if (!array) {
        return NULL;
    }

    array->count    = 0;
    array->capacity = capacity;
    if (capacity > 0) {
        // Check for overflow before calculating allocation size
        if (capacity > SIZE_MAX / sizeof(mvn_val_t)) {
            MVN_DS_FREE(array);
            fprintf(stderr, "[MVN_DS_ARRAY] Initial capacity overflow.\n");
            return NULL;
        }
        size_t allocation_size = capacity * sizeof(mvn_val_t);
        array->data            = (mvn_val_t *)MVN_DS_MALLOC(allocation_size);
        if (!array->data) {
            MVN_DS_FREE(array);
            return NULL;
        }
        // Initialize to NULL
        for (size_t index = 0; index < capacity; ++index) {
            array->data[index] = mvn_val_null();
        }
    } else {
        array->data = NULL; // No initial allocation if capacity is 0
    }
    return array;
}

/**
 * @brief Creates a new, empty dynamic array with a default initial capacity.
 * Uses MVN_DS_ARRAY_INITIAL_CAPACITY defined in the header.
 * @return A pointer to the new mvn_array_t, or NULL on allocation failure.
 */
mvn_array_t *mvn_array_new(void)
{
    // Use MVN_DS_ARRAY_INITIAL_CAPACITY by default
    return mvn_array_new_with_capacity(MVN_DS_ARRAY_INITIAL_CAPACITY);
}

/**
 * @brief Frees the memory associated with a dynamic array, including all contained values.
 * Iterates through the array elements and calls mvn_val_free on each one before
 * freeing the data buffer and the array structure itself.
 * @param array The array to free. Does nothing if NULL.
 */
void mvn_array_free(mvn_array_t *array)
{
    if (!array) {
        return;
    }
    // Free contained values if data buffer exists
    if (array->data) {
        for (size_t index = 0; index < array->count; index++) {
            mvn_val_free(&array->data[index]);
        }
        MVN_DS_FREE(array->data); // Free the data buffer
    }
    MVN_DS_FREE(array); // Free the array struct itself
}

/**
 * @brief Appends a value to the end of the array.
 * The array takes ownership of the value if it's a dynamic type (STRING, ARRAY, HASHMAP).
 * Resizes the array using mvn_array_ensure_capacity if necessary.
 * @param array The array to append to. Must not be NULL.
 * @param value The value to append. Ownership is transferred to the array.
 * @return true if successful, false on allocation failure or invalid input.
 */
bool mvn_array_push(mvn_array_t *array, mvn_val_t value)
{
    if (!array) {
        // If array is NULL, we cannot take ownership, so free the value if needed.
        mvn_val_free(&value);
        return false;
    }
    if (!mvn_array_ensure_capacity(array)) {
        // If resize fails, we cannot take ownership, so free the value if needed.
        mvn_val_free(&value);
        return false;
    }
    // Place the value (transfers ownership) and increment count
    array->data[array->count] = value;
    array->count++;
    return true;
}

/**
 * @brief Retrieves a pointer to the value at a specific index.
 * Does not transfer ownership. Returns NULL if the index is out of bounds or array is NULL.
 * @param array The array to access. Can be NULL.
 * @param index The index of the element to retrieve.
 * @return A pointer to the mvn_val_t at the index, or NULL if array is NULL or index is out of
 * bounds.
 */
mvn_val_t *mvn_array_get(const mvn_array_t *array, size_t index)
{
    if (!array || index >= array->count) {
        return NULL;
    }
    return &array->data[index];
}

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
bool mvn_array_set(mvn_array_t *array, size_t index, mvn_val_t value)
{
    if (!array || index >= array->count) {
        // If index is invalid, we cannot take ownership, so free the value if needed.
        mvn_val_free(&value);
        return false;
    }
    // Free the old value at the index before overwriting
    mvn_val_free(&array->data[index]);
    // Assign the new value (transfers ownership)
    array->data[index] = value;
    return true;
}
