/*
 * Copyright (c) 2024 Jake Larson
 */
#include "mvn_ds/mvn_ds_arr.h"

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
static void *mvn_arr_reallocate(void *pointer, size_t new_size)
{
    if (new_size == 0) {
        MVN_DS_FREE(pointer);
        return NULL;
    }
    void *result = MVN_DS_REALLOC(pointer, new_size);
    if (result == NULL && new_size > 0) {
        fprintf(stderr, "[MVN_DS_ARR] Memory reallocation failed!\n");
    }
    return result;
}

/**
 * @internal
 * @brief Ensures array has enough capacity, reallocating if necessary.
 * @param array The array to check/grow. Must not be NULL.
 * @return true if successful (or no resize needed), false on allocation failure.
 */
static bool mvn_arr_ensure_capacity(mvn_arr_t *array)
{
    assert(array != NULL);
    if (array->count < array->capacity) {
        return true; // Enough space
    }
    size_t old_capacity = array->capacity;
    size_t new_capacity = old_capacity < MVN_DS_ARR_INITIAL_CAPACITY ?
                              MVN_DS_ARR_INITIAL_CAPACITY :
                              old_capacity * MVN_DS_ARR_GROWTH_FACTOR;

    // Check for potential overflow during growth calculation
    if (new_capacity < old_capacity && old_capacity > 0) {
        fprintf(stderr, "[MVN_DS_ARR] Array capacity overflow during resize calculation.\n");
        if (SIZE_MAX - 1 < array->count) {
            return false;
        }
        new_capacity = array->count + 1;
        if (new_capacity <= old_capacity) {
            return false;
        }
    } else if (new_capacity == 0 && old_capacity == 0) {
        new_capacity = MVN_DS_ARR_INITIAL_CAPACITY;
    }

    // Check for overflow before calculating allocation size
    if (new_capacity > SIZE_MAX / sizeof(mvn_val_t)) {
        fprintf(stderr, "[MVN_DS_ARR] Array capacity overflow calculating allocation size.\n");
        return false;
    }
    size_t allocation_size = new_capacity * sizeof(mvn_val_t);

    mvn_val_t *new_data = (mvn_val_t *)mvn_arr_reallocate(array->data, allocation_size);
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
 * @return A pointer to the new mvn_arr_t, or NULL on allocation failure.
 */
mvn_arr_t *mvn_arr_new_capacity(size_t capacity)
{
    mvn_arr_t *array = (mvn_arr_t *)MVN_DS_MALLOC(sizeof(mvn_arr_t));
    if (!array) {
        return NULL;
    }

    array->count    = 0;
    array->capacity = capacity;
    if (capacity > 0) {
        if (capacity > SIZE_MAX / sizeof(mvn_val_t)) {
            MVN_DS_FREE(array);
            fprintf(stderr, "[MVN_DS_ARR] Initial capacity overflow.\n");
            return NULL;
        }
        array->data = (mvn_val_t *)MVN_DS_CALLOC(capacity, sizeof(mvn_val_t));
        if (!array->data) {
            MVN_DS_FREE(array);
            return NULL;
        }
    } else {
        array->data = NULL;
    }
    return array;
}

/**
 * @brief Creates a new, empty dynamic array with a default initial capacity.
 * Uses MVN_DS_ARR_INITIAL_CAPACITY defined in the header.
 * @return A pointer to the new mvn_arr_t, or NULL on allocation failure.
 */
mvn_arr_t *mvn_arr_new(void)
{
    return mvn_arr_new_capacity(MVN_DS_ARR_INITIAL_CAPACITY);
}

/**
 * @brief Frees the memory associated with a dynamic array, including all contained values.
 * Iterates through the array elements and calls mvn_val_free on each one before
 * freeing the data buffer and the array structure itself.
 * @param array The array to free. Does nothing if NULL.
 */
void mvn_arr_free(mvn_arr_t *array)
{
    if (!array) {
        return;
    }
    if (array->data) {
        for (size_t index = 0; index < array->count; index++) {
            mvn_val_free(&array->data[index]);
        }
        MVN_DS_FREE(array->data);
    }
    MVN_DS_FREE(array);
}

/**
 * @brief Appends a value to the end of the array.
 * The array takes ownership of the value if it's a dynamic type (STRING, ARRAY, HASHMAP).
 * Resizes the array using mvn_arr_ensure_capacity if necessary.
 * @param array The array to append to. Must not be NULL.
 * @param value The value to append. Ownership is transferred to the array.
 * @return true if successful, false on allocation failure or invalid input.
 */
bool mvn_arr_push(mvn_arr_t *array, mvn_val_t value)
{
    if (!array) {
        mvn_val_free(&value);
        return false;
    }
    if (!mvn_arr_ensure_capacity(array)) {
        mvn_val_free(&value);
        return false;
    }
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
mvn_val_t *mvn_arr_get(const mvn_arr_t *array, size_t index)
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
bool mvn_arr_set(mvn_arr_t *array, size_t index, mvn_val_t value)
{
    if (!array || index >= array->count) {
        mvn_val_free(&value);
        return false;
    }
    mvn_val_free(&array->data[index]);
    array->data[index] = value;
    return true;
}

/**
 * @brief Returns the number of elements in the array.
 * @param array The array. Can be NULL.
 * @return The number of elements, or 0 if the array is NULL.
 */
size_t mvn_arr_count(const mvn_arr_t *array)
{
    return array ? array->count : 0;
}

/**
 * @brief Returns the current capacity of the array.
 * @param array The array. Can be NULL.
 * @return The capacity, or 0 if the array is NULL.
 */
size_t mvn_arr_capacity(const mvn_arr_t *array)
{
    return array ? array->capacity : 0;
}

/**
 * @brief Checks if the array is empty.
 * @param array The array. Can be NULL.
 * @return true if the array is empty (or NULL), false otherwise.
 */
bool mvn_arr_is_empty(const mvn_arr_t *array)
{
    return !array || array->count == 0;
}
