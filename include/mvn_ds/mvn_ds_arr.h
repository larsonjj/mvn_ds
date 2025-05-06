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
mvn_arr_t *mvn_arr_new_cap(size_t capacity);

// Frees the memory associated with a dynamic array, including all contained values.
void mvn_arr_free(mvn_arr_t *array);

// Appends a value to the end of the array, taking ownership if dynamic.
bool mvn_arr_push(mvn_arr_t *array, mvn_val_t value);

// Retrieves a pointer to the value at a specific index (no ownership transfer).
mvn_val_t *mvn_arr_get(const mvn_arr_t *array, size_t index);

// Sets the value at a specific index, freeing the old value and taking ownership of the new.
bool mvn_arr_set(mvn_arr_t *array, size_t index, mvn_val_t value);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MVN_DS_ARR_H */
