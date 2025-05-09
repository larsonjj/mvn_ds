/*
 * Copyright (c) 2024 Jake Larson
 */
#ifndef MVN_DS_ARR_H
#define MVN_DS_ARR_H

#include "mvn_ds_types.h" // Include the structure definitions

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// Default initial capacity for new arrays created with mvn_arr_new()
#define MVN_DS_ARR_INITIAL_CAPACITY 8
// Factor by which the array capacity grows when resizing
#define MVN_DS_ARR_GROWTH_FACTOR 2

// --- Array Operations ---

// Creates a new, empty dynamic array with a default initial capacity.
mvn_arr_t *mvn_arr_new(void);

// Creates a new, empty dynamic array with a specific initial capacity.
mvn_arr_t *mvn_arr_new_capacity(size_t capacity);

// Frees the memory associated with a dynamic array, including all contained values.
void mvn_arr_free(mvn_arr_t *array);

// Appends a value to the end of the array, taking ownership if dynamic.
bool mvn_arr_push(mvn_arr_t *array, mvn_val_t value);

// Retrieves a pointer to the value at a specific index (no ownership transfer).
mvn_val_t *mvn_arr_get(const mvn_arr_t *array, size_t index);

// Sets the value at a specific index, freeing the old value and taking ownership of the new.
bool mvn_arr_set(mvn_arr_t *array, size_t index, mvn_val_t value);

// Returns the number of elements in the array.
size_t mvn_arr_count(const mvn_arr_t *array);

// Returns the current capacity of the array.
size_t mvn_arr_capacity(const mvn_arr_t *array);

// Checks if the array is empty.
bool mvn_arr_is_empty(const mvn_arr_t *array);

// Sorts the array in-place using the provided comparison function.
// The comparison function should return <0 if a < b, 0 if a == b, >0 if a > b.
bool mvn_arr_sort(mvn_arr_t *array, int (*compare_func)(const mvn_val_t *a, const mvn_val_t *b));

// Creates a new array containing elements for which predicate_func returns true.
// The new array owns deep copies of the selected elements.
// Returns NULL if the input array or predicate_func is NULL, or on allocation failure.
mvn_arr_t *mvn_arr_filter(const mvn_arr_t *array, bool (*predicate_func)(const mvn_val_t *value));

// Creates a new array by applying transform_func to each element of the original array.
// The new array owns the transformed elements.
// Returns NULL if the input array or transform_func is NULL, or on allocation failure.
mvn_arr_t *mvn_arr_map(const mvn_arr_t *array, mvn_val_t (*transform_func)(const mvn_val_t *value));

// Removes the last element from the array and returns it. Caller takes ownership.
// Returns mvn_val_null() if the array is empty or NULL.
mvn_val_t mvn_arr_pop(mvn_arr_t *array);

// Removes the element at the specified index. Elements are shifted.
// Returns true on success, false if array is NULL or index is out of bounds.
bool mvn_arr_remove_at(mvn_arr_t *array, size_t index);

// Inserts a value at the specified index. Elements are shifted. Takes ownership of value.
// Returns true on success, false if array is NULL, index is out of bounds (beyond count), or on
// allocation failure.
bool mvn_arr_insert_at(mvn_arr_t *array, size_t index, mvn_val_t value);

// Removes all elements from the array. Dynamic elements are freed. Capacity remains.
void mvn_arr_clear(mvn_arr_t *array);

// Reduces array capacity to fit its current number of elements.
// Returns true on success, false on failure (e.g., reallocation failure).
bool mvn_arr_shrink_to_fit(mvn_arr_t *array);

// Checks if the array contains a specific value.
// Uses mvn_val_equal for comparison.
bool mvn_arr_contains(const mvn_arr_t *array, const mvn_val_t *value_to_find);

// Finds the first occurrence of a value in the array, starting from a given index.
// Returns a non-owning pointer to the found mvn_val_t, or NULL if not found or on invalid input.
// Uses mvn_val_equal for comparison.
mvn_val_t *mvn_arr_find(const mvn_arr_t *array, const mvn_val_t *value_to_find, size_t start_index);

// Finds the last occurrence of a value in the array.
// Returns a non-owning pointer to the found mvn_val_t, or NULL if not found or on invalid input.
// Uses mvn_val_equal for comparison.
mvn_val_t *mvn_arr_find_last(const mvn_arr_t *array, const mvn_val_t *value_to_find);

// Finds the first index of a value in the array, starting from a given index.
// Uses mvn_val_equal for comparison.
// Returns the index of the first occurrence, or -1 if not found or on invalid input.
ptrdiff_t
mvn_arr_index_of(const mvn_arr_t *array, const mvn_val_t *value_to_find, size_t start_index);

// Finds the last index of a value in the array.
// Uses mvn_val_equal for comparison.
// Returns the index of the last occurrence, or -1 if not found or on invalid input.
ptrdiff_t mvn_arr_last_index_of(const mvn_arr_t *array, const mvn_val_t *value_to_find);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MVN_DS_ARR_H */
