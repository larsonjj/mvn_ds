/*
 * Copyright (c) 2024 Jake Larson
 */
#include "mvn_ds/mvn_ds_arr.h"

#include "mvn_ds/mvn_ds.h"       // Provides mvn_val_null, mvn_val_free
#include "mvn_ds/mvn_ds_utils.h" // Provides memory macros (MVN_DS_*)

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h> // For SIZE_MAX, qsort
#include <string.h> // For memmove

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

/**
 * @brief Sorts the array in-place using the provided comparison function.
 * The comparison function should follow the standard signature for qsort:
 * return <0 if a < b, 0 if a == b, >0 if a > b.
 * @param array The array to sort. Must not be NULL.
 * @param compare_func The comparison function. Must not be NULL.
 * @return true if sorting was initiated (array and func not NULL), false otherwise.
 *         Note: qsort itself does not return a status. This function indicates if the call was
 * made.
 */
bool mvn_arr_sort(mvn_arr_t *array, int (*compare_func)(const mvn_val_t *a, const mvn_val_t *b))
{
    if (!array || !compare_func) {
        return false;
    }
    if (array->count > 1) { // qsort is not needed for 0 or 1 elements
        qsort(array->data,
              array->count,
              sizeof(mvn_val_t),
              (int (*)(const void *, const void *))compare_func);
    }
    return true;
}

/**
 * @brief Creates a new array containing elements for which predicate_func returns true.
 * The new array owns deep copies of the selected elements.
 * @param array The source array. Can be NULL.
 * @param predicate_func Function to test each element. Must not be NULL.
 * @return A new mvn_arr_t containing filtered elements, or NULL if input array or
 *         predicate_func is NULL, or on allocation failure during copying or array creation.
 */
mvn_arr_t *mvn_arr_filter(const mvn_arr_t *array, bool (*predicate_func)(const mvn_val_t *value))
{
    if (!array || !predicate_func) {
        return NULL;
    }

    // Create a new array, initially with a capacity that might be an overestimate
    // or start small and grow. Starting with original count is a reasonable guess.
    mvn_arr_t *filtered_array_ptr =
        mvn_arr_new_capacity(array->count > 0 ? array->count : MVN_DS_ARR_INITIAL_CAPACITY);
    if (!filtered_array_ptr) {
        return NULL; // Allocation failure for the new array structure
    }

    for (size_t i = 0; i < array->count; ++i) {
        if (array->data && predicate_func(&array->data[i])) {
            mvn_val_t copied_value = mvn_val_deep_copy(&array->data[i]);
            // Check if deep copy failed (e.g. returned MVN_VAL_NULL due to inner allocation
            // failure)
            if (copied_value.type == MVN_VAL_NULL && array->data[i].type != MVN_VAL_NULL) {
                mvn_arr_free(filtered_array_ptr); // Free partially filled array
                return NULL;                      // Deep copy failed
            }

            if (!mvn_arr_push(filtered_array_ptr, copied_value)) {
                mvn_val_free(&copied_value);      // Free the copied value that failed to push
                mvn_arr_free(filtered_array_ptr); // Free the partially filled array
                return NULL;                      // Push failed
            }
        }
    }
    return filtered_array_ptr;
}

/**
 * @brief Creates a new array by applying transform_func to each element of the original array.
 * The new array owns the transformed elements. The transform_func is responsible for
 * allocating any dynamic memory for the mvn_val_t it returns.
 * @param array The source array. Can be NULL.
 * @param transform_func Function to transform each element. Must not be NULL.
 * @return A new mvn_arr_t containing transformed elements, or NULL if input array or
 *         transform_func is NULL, or on allocation failure during array creation or push.
 */
mvn_arr_t *mvn_arr_map(const mvn_arr_t *array, mvn_val_t (*transform_func)(const mvn_val_t *value))
{
    if (!array || !transform_func) {
        return NULL;
    }

    mvn_arr_t *mapped_array_ptr = mvn_arr_new_capacity(array->count);
    if (!mapped_array_ptr) {
        return NULL; // Allocation failure for the new array structure
    }

    for (size_t i = 0; i < array->count; ++i) {
        if (array->data) {
            mvn_val_t transformed_value = transform_func(&array->data[i]);
            // The transformed_value is now owned by this scope.
            // If push fails, we must free it.
            if (!mvn_arr_push(mapped_array_ptr, transformed_value)) {
                mvn_val_free(&transformed_value); // Free the value that failed to push
                mvn_arr_free(mapped_array_ptr);   // Free the partially filled array
                return NULL;                      // Push failed
            }
        }
    }
    return mapped_array_ptr;
}

/**
 * @brief Removes the last element from the array and returns it.
 * The caller takes ownership of the returned mvn_val_t.
 * If the array is empty or NULL, it returns a mvn_val_t of type MVN_VAL_NULL.
 * @param array The array to pop from.
 * @return The popped mvn_val_t.
 */
mvn_val_t mvn_arr_pop(mvn_arr_t *array)
{
    if (!array || array->count == 0) {
        return mvn_val_null();
    }

    array->count--;
    mvn_val_t popped_value    = array->data[array->count];
    array->data[array->count] = mvn_val_null(); // Prevent double free if array is freed later
    return popped_value;
}

/**
 * @brief Removes the element at the specified index.
 * Elements after the removed one are shifted down. The removed mvn_val_t is freed.
 * @param array The array to modify.
 * @param index The index of the element to remove.
 * @return true if successful, false if array is NULL or index is out of bounds.
 */
bool mvn_arr_remove_at(mvn_arr_t *array, size_t index)
{
    if (!array || index >= array->count) {
        return false;
    }

    mvn_val_free(&array->data[index]); // Free the element being removed

    // Shift elements down if it's not the last element
    if (index < array->count - 1) {
        memmove(&array->data[index],
                &array->data[index + 1],
                (array->count - 1 - index) * sizeof(mvn_val_t));
    }

    array->count--;
    // Set the now-unused slot at the end to NULL to be safe
    if (array->capacity > 0) { // Ensure data is not NULL
        array->data[array->count] = mvn_val_null();
    }
    return true;
}

/**
 * @brief Inserts a value at the specified index in the array.
 * Existing elements from the index onwards are shifted up. The array takes ownership of the value.
 * @param array The array to modify.
 * @param index The index at which to insert the value. Must be <= array->count.
 * @param value The mvn_val_t to insert.
 * @return true if successful, false if array is NULL, index is out of bounds, or on allocation
 * failure.
 */
bool mvn_arr_insert_at(mvn_arr_t *array, size_t index, mvn_val_t value)
{
    if (!array || index > array->count) { // Allow insertion at the end (index == count)
        mvn_val_free(&value);             // Free value if insertion is invalid
        return false;
    }

    if (!mvn_arr_ensure_capacity(array)) {
        mvn_val_free(&value); // Free value if capacity cannot be ensured
        return false;
    }

    // Shift elements up if inserting before the end
    if (index < array->count) {
        memmove(&array->data[index + 1],
                &array->data[index],
                (array->count - index) * sizeof(mvn_val_t));
    }

    array->data[index] = value;
    array->count++;
    return true;
}

/**
 * @brief Removes all elements from the array.
 * Dynamic elements are freed. The capacity of the array remains unchanged.
 * @param array The array to clear.
 */
void mvn_arr_clear(mvn_arr_t *array)
{
    if (!array) {
        return;
    }
    for (size_t i = 0; i < array->count; ++i) {
        mvn_val_free(&array->data[i]);
        // No need to set to mvn_val_null here as count will be 0
    }
    array->count = 0;
}

/**
 * @brief Reduces the array's capacity to match its current number of elements.
 * If the count is 0, the internal data buffer may be freed.
 * @param array The array to shrink.
 * @return true if successful or no action needed, false on reallocation failure.
 */
bool mvn_arr_shrink_to_fit(mvn_arr_t *array)
{
    if (!array) {
        return false; // Or true if NULL array is considered "shrunk"
    }
    if (array->capacity == array->count) {
        return true; // Already at optimal capacity
    }

    if (array->count == 0) {
        MVN_DS_FREE(array->data);
        array->data     = NULL;
        array->capacity = 0;
        return true;
    }

    // Reallocate to the exact size needed
    size_t     new_allocation_size = array->count * sizeof(mvn_val_t);
    mvn_val_t *new_data_ptr = (mvn_val_t *)mvn_arr_reallocate(array->data, new_allocation_size);

    if (!new_data_ptr) {
        // Reallocation failed, original data is still valid (as per realloc behavior)
        return false;
    }

    array->data     = new_data_ptr;
    array->capacity = array->count;
    return true;
}

/**
 * @brief Finds the first index of a value in the array, starting from a given index.
 *
 * Iterates through the array starting from `start_index` and compares each element
 * with `value_to_find` using `mvn_val_equal`.
 *
 * @param array The array to search. Must not be NULL.
 * @param value_to_find Pointer to the `mvn_val_t` to search for. Must not be NULL.
 * @param start_index The index from which to begin the search. Must be less than `array->count`
 *                    if the array is not empty. If `array->count` is 0, `start_index` must be 0.
 * @return The `ptrdiff_t` index of the first occurrence of the value if found;
 *         otherwise, -1. Returns -1 if `array` or `value_to_find` is NULL,
 *         or if `start_index` is out of bounds.
 */
ptrdiff_t
mvn_arr_index_of(const mvn_arr_t *array, const mvn_val_t *value_to_find, size_t start_index)
{
    if (!array || !value_to_find) {
        return -1;
    }

    // Handle edge case for empty array: if start_index is 0, it's valid but will find nothing.
    // If start_index is > 0 for an empty array, it's out of bounds.
    if (array->count == 0) {
        // If array is empty, only start_index 0 is "valid" in the sense that it doesn't
        // immediately indicate an out-of-bounds access attempt on a non-empty array.
        // In either case for an empty array, the value won't be found.
        return -1;
    }

    // For a non-empty array, start_index must be within bounds.
    if (start_index >= array->count) {
        return -1;
    }

    for (size_t idx = start_index; idx < array->count; ++idx) {
        // Ensure array->data is valid before dereferencing, though mvn_arr_get/set usually ensure
        // this if count > 0. However, direct access warrants a check if data could be NULL with
        // count > 0 (which shouldn't happen with current mvn_arr_new_capacity logic).
        if (array->data && mvn_val_equal(&array->data[idx], value_to_find)) {
            return (ptrdiff_t)idx;
        }
    }
    return -1; // Not found
}

/**
 * @brief Finds the last index of a value in the array.
 *
 * Iterates through the array in reverse order and compares each element
 * with `value_to_find` using `mvn_val_equal`.
 *
 * @param array The array to search. Must not be NULL.
 * @param value_to_find Pointer to the `mvn_val_t` to search for. Must not be NULL.
 * @return The `ptrdiff_t` index of the last occurrence of the value if found;
 *         otherwise, -1. Returns -1 if `array` or `value_to_find` is NULL,
 *         or if the array is empty.
 */
ptrdiff_t mvn_arr_last_index_of(const mvn_arr_t *array, const mvn_val_t *value_to_find)
{
    if (!array || !value_to_find || array->count == 0) {
        return -1;
    }

    // Iterate backwards from the last element
    // Loop condition uses idx >= 0 which is safe for ptrdiff_t
    for (ptrdiff_t idx = (ptrdiff_t)array->count - 1; idx >= 0; --idx) {
        // Similar check for array->data as in mvn_arr_index_of
        if (array->data && mvn_val_equal(&array->data[idx], value_to_find)) {
            return idx;
        }
    }
    return -1; // Not found
}

/**
 * @brief Checks if the array contains a specific value.
 * Uses mvn_val_equal for comparison.
 * @param array The array to check.
 * @param value_to_find Pointer to the value to search for.
 * @return true if the value is found, false otherwise or if inputs are invalid.
 */
bool mvn_arr_contains(const mvn_arr_t *array, const mvn_val_t *value_to_find)
{
    if (!array || !value_to_find) {
        return false;
    }
    return mvn_arr_index_of(array, value_to_find, 0) != -1;
}

/**
 * @brief Finds the first occurrence of a value in the array, starting from a given index.
 * Returns a non-owning pointer to the found mvn_val_t.
 * Uses mvn_val_equal for comparison.
 * @param array The array to search.
 * @param value_to_find Pointer to the value to search for.
 * @param start_index The index from which to start searching.
 * @return A pointer to the found mvn_val_t within the array, or NULL if not found,
 *         array is NULL, value_to_find is NULL, or start_index is out of bounds.
 */
mvn_val_t *mvn_arr_find(const mvn_arr_t *array, const mvn_val_t *value_to_find, size_t start_index)
{
    if (!array || !value_to_find || !array->data) { // Check array->data as well
        return NULL;
    }
    // If start_index is valid and array has elements, or if array is empty and start_index is 0
    if (start_index < array->count || (array->count == 0 && start_index == 0)) {
        for (size_t i = start_index; i < array->count; ++i) {
            if (mvn_val_equal(&array->data[i], value_to_find)) {
                return &array->data[i];
            }
        }
    }
    return NULL;
}

/**
 * @brief Finds the last occurrence of a value in the array.
 * Returns a non-owning pointer to the found mvn_val_t.
 * Uses mvn_val_equal for comparison.
 * @param array The array to search.
 * @param value_to_find Pointer to the value to search for.
 * @return A pointer to the found mvn_val_t within the array, or NULL if not found,
 *         array is NULL, value_to_find is NULL, or array is empty.
 */
mvn_val_t *mvn_arr_find_last(const mvn_arr_t *array, const mvn_val_t *value_to_find)
{
    if (!array || !value_to_find || array->count == 0 || !array->data) {
        return NULL;
    }

    for (ptrdiff_t i = (ptrdiff_t)array->count - 1; i >= 0; --i) {
        if (mvn_val_equal(&array->data[i], value_to_find)) {
            return &array->data[i];
        }
    }
    return NULL;
}
