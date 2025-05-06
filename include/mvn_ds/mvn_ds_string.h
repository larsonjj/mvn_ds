/*
 * Copyright (c) 2024 Jake Larson
 */
#ifndef MVN_DS_STRING_H
#define MVN_DS_STRING_H

#include "mvn_ds_types.h" // Include the structure definitions

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h> // For uint32_t used in hash

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @brief Default initial capacity for new strings created with mvn_string_new(). */
#define MVN_DS_STRING_INITIAL_CAPACITY 8
/** @brief Factor by which the string capacity grows when resizing. */
#define MVN_DS_STRING_GROWTH_FACTOR 2

// --- String Operations ---

// Creates a new string by copying a C string.
mvn_string_t *mvn_string_new(const char *chars);

// Creates a new string with a specific initial capacity.
mvn_string_t *mvn_string_new_with_capacity(size_t capacity);

// Frees the memory associated with a string.
void mvn_string_free(mvn_string_t *string_ptr);

// Appends a C string to an mvn_string_t.
bool mvn_string_append_cstr(mvn_string_t *string_ptr, const char *chars);

// Appends another mvn_string_t to an mvn_string_t.
bool mvn_string_append(mvn_string_t *dest_ptr, const mvn_string_t *src_ptr);

// Compares two mvn_string_t strings for equality.
bool mvn_string_equal(const mvn_string_t *str1_ptr, const mvn_string_t *str2_ptr);

// Compares an mvn_string_t with a C string for equality.
bool mvn_string_equal_cstr(const mvn_string_t *str1_ptr, const char *cstr2);

// Calculates a hash value for the string (FNV-1a algorithm).
uint32_t mvn_string_hash(const mvn_string_t *string_ptr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MVN_DS_STRING_H */
