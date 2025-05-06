/*
 * Copyright (c) 2024 Jake Larson
 */
#ifndef MVN_DS_H
#define MVN_DS_H

#include "mvn_ds_types.h" // Include all structure definitions

// Include component function declarations
#include "mvn_ds_array.h"
#include "mvn_ds_hmap.h"
#include "mvn_ds_string.h"

// Include basic stdlib headers needed by users of mvn_val_t directly
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>  // For mvn_val_print potentially using printf
#include <stdlib.h> // For size_t

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// --- Value Constructors ---
mvn_val_t mvn_val_null(void);
mvn_val_t mvn_val_bool(bool b);
mvn_val_t mvn_val_i32(int32_t i32);
mvn_val_t mvn_val_i64(int64_t i64);
mvn_val_t mvn_val_f32(float f32);
mvn_val_t mvn_val_f64(double f64);
mvn_val_t mvn_val_string(const char *chars);      // Creates a new owned string
mvn_val_t mvn_val_string_take(mvn_string_t *str); // Takes ownership of an existing string
mvn_val_t mvn_val_array(void);                    // Creates a new empty owned array
mvn_val_t mvn_val_array_take(mvn_array_t *arr);   // Takes ownership of an existing array
mvn_val_t mvn_val_hmap(void);                     // Creates a new empty owned hash map
mvn_val_t mvn_val_hmap_take(mvn_hmap_t *hmap);    // Takes ownership of an existing map

// --- Value Operations ---
/**
 * @brief Frees the resources owned by a mvn_val_t.
 * If the value type is STRING, ARRAY, or HASHMAP, it frees the associated
 * dynamic structure recursively. For other types, it does nothing.
 * Resets the value to MVN_VAL_NULL after freeing.
 * @param value Pointer to the value to free. Does nothing if NULL.
 */
void mvn_val_free(mvn_val_t *value);

/**
 * @brief Prints a representation of the value to stdout (for debugging).
 * @param value Pointer to the value to print. Handles NULL gracefully.
 */
void mvn_val_print(const mvn_val_t *value);

// --- Component Operations ---
// Declarations are now in their respective headers (mvn_ds_string.h, etc.)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // MVN_DS_H
