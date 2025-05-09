/*
 * Copyright (c) 2024 Jake Larson
 */
#ifndef MVN_DS_H
#define MVN_DS_H

#include "mvn_ds_types.h" // Include all structure definitions

// Include component function declarations
#include "mvn_ds_arr.h"
#include "mvn_ds_hmap.h"
#include "mvn_ds_str.h"

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
mvn_val_t mvn_val_i8(int8_t i8_val);
mvn_val_t mvn_val_i16(int16_t i16_val);
mvn_val_t mvn_val_i32(int32_t i32);
mvn_val_t mvn_val_i64(int64_t i64);
mvn_val_t mvn_val_u8(uint8_t u8_val);
mvn_val_t mvn_val_u16(uint16_t u16_val);
mvn_val_t mvn_val_u32(uint32_t u32);
mvn_val_t mvn_val_u64(uint64_t u64);
mvn_val_t mvn_val_f32(float f32);
mvn_val_t mvn_val_f64(double f64);
mvn_val_t mvn_val_char(char c);
mvn_val_t mvn_val_ptr(void *ptr_val);
mvn_val_t mvn_val_str(const char *chars);      // Creates a new owned string
mvn_val_t mvn_val_str_take(mvn_str_t *str);    // Takes ownership of an existing string
mvn_val_t mvn_val_arr(void);                   // Creates a new empty owned array
mvn_val_t mvn_val_arr_take(mvn_arr_t *arr);    // Takes ownership of an existing array
mvn_val_t mvn_val_hmap(void);                  // Creates a new empty owned hash map
mvn_val_t mvn_val_hmap_take(mvn_hmap_t *hmap); // Takes ownership of an existing map

// --- Value Operations ---
// Frees the resources owned by a mvn_val_t.
void mvn_val_free(mvn_val_t *value);

// Prints a representation of the value to stdout (for debugging).
void mvn_val_print(const mvn_val_t *value);

// Compares two mvn_val_t values for equality.
bool mvn_val_equal(const mvn_val_t *val_one, const mvn_val_t *val_two);

// Creates a deep copy of a mvn_val_t.
// For dynamic types (STRING, ARRAY, HASHMAP), this means new allocations and copying content.
// For PTR type, the pointer value is copied, not the data it points to.
mvn_val_t mvn_val_deep_copy(const mvn_val_t *original_value);

// Compares two mvn_val_t values. Useful for sorting.
// Returns <0 if val_one < val_two, 0 if equal, >0 if val_one > val_two.
// Comparison order between different types is defined (e.g., NULL < BOOL < I32 < STRING ...).
int mvn_val_compare(const mvn_val_t *val_one, const mvn_val_t *val_two);

// Converts a mvn_val_type_t enum to its string representation.
const char *mvn_val_type_to_str(mvn_val_type_t type);

// --- Component Operations ---
// Declarations are now in their respective headers (mvn_ds_str.h, etc.)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // MVN_DS_H
