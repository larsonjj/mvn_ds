/*
 * Copyright (c) 2024 Jake Larson
 */
#include "mvn_ds/mvn_ds_array.h"

#include "mvn_ds/mvn_ds.h"       // Provides mvn_val_null, mvn_val_free
#include "mvn_ds/mvn_ds_utils.h" // Provides mvn_reallocate

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h> // For SIZE_MAX

// --- Internal Helper Functions ---

/**
 * @internal
 * @brief Ensures array has enough capacity, reallocating if necessary.
 * @param array The array to check/grow.
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

    mvn_val_t *new_data = (mvn_val_t *)mvn_reallocate( // This now calls the header version
        array->data,
        old_capacity * sizeof(mvn_val_t),
        allocation_size);
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

mvn_array_t *mvn_array_new(void)
{
    // Use MVN_DS_ARRAY_INITIAL_CAPACITY by default
    return mvn_array_new_with_capacity(MVN_DS_ARRAY_INITIAL_CAPACITY);
}

void mvn_array_free(mvn_array_t *array)
{
    if (!array) {
        return;
    }
    // Free contained values first
    // Iterate up to capacity because ensure_capacity initializes slots up to capacity
    if (array->data) {
        for (size_t index = 0; index < array->capacity; index++) {
            mvn_val_free(&array->data[index]);
        }
        MVN_DS_FREE(array->data); // Free the value buffer
    }
    MVN_DS_FREE(array); // Free the struct itself
}

bool mvn_array_push(mvn_array_t *array, mvn_val_t value)
{
    if (!array) {
        // Cannot push to NULL array, free incoming value if needed
        mvn_val_free(&value);
        return false;
    }
    if (!mvn_array_ensure_capacity(array)) {
        // If resize failed, we might own the value now but can't store it.
        // Free it to prevent leaks.
        mvn_val_free(&value);
        return false;
    }
    // Place the value (transfers ownership)
    // Ensure data is not NULL (should be handled by ensure_capacity)
    assert(array->data != NULL || array->capacity == 0);
    // If capacity was 0, ensure_capacity should have allocated data
    assert(array->data != NULL || array->count == 0);

    array->data[array->count++] = value;
    return true;
}

mvn_val_t *mvn_array_get(const mvn_array_t *array, size_t index)
{
    if (!array || !array->data || index >= array->count) {
        return NULL;
    }
    // Const cast is generally acceptable here for returning a non-const pointer
    // from a const structure pointer, assuming the caller respects const-correctness
    // if they originally had a const mvn_array_t*.
    return &((mvn_array_t *)array)->data[index];
}

bool mvn_array_set(mvn_array_t *array, size_t index, mvn_val_t value)
{
    if (!array || !array->data || index >= array->count) {
        // Index out of bounds or invalid array/data.
        // Free the incoming value as it won't be stored.
        mvn_val_free(&value);
        return false;
    }
    // Free the old value at the index before overwriting
    mvn_val_free(&array->data[index]);
    // Assign the new value (transfers ownership)
    array->data[index] = value;
    return true;
}
