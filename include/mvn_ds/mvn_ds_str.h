/*
 * Copyright (c) 2024 Jake Larson
 */
#ifndef MVN_DS_STR_H
#define MVN_DS_STR_H

#include "mvn_ds_types.h" // Include the structure definitions

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h> // For uint32_t used in hash

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @brief Default initial capacity for new strings created with mvn_str_new(). */
#define MVN_DS_STR_INITIAL_CAPACITY 8
/** @brief Factor by which the string capacity grows when resizing. */
#define MVN_DS_STR_GROWTH_FACTOR 2

// --- String Operations ---

// Creates a new string by copying a C string.
mvn_str_t *mvn_str_new(const char *chars);

// Creates a new string with a specific initial capacity.
mvn_str_t *mvn_str_new_with_capacity(size_t capacity);

// Frees the memory associated with a string.
void mvn_str_free(mvn_str_t *string_ptr);

// Appends a C string to an mvn_str_t.
bool mvn_str_append_cstr(mvn_str_t *string_ptr, const char *chars);

// Appends another mvn_str_t to an mvn_str_t.
bool mvn_str_append(mvn_str_t *dest_ptr, const mvn_str_t *src_ptr);

// Compares two mvn_str_t strings for equality.
bool mvn_str_equal(const mvn_str_t *str1_ptr, const mvn_str_t *str2_ptr);

// Compares an mvn_str_t with a C string for equality.
bool mvn_str_equal_cstr(const mvn_str_t *str1_ptr, const char *cstr2);

// Calculates a hash value for the string (FNV-1a algorithm).
uint32_t mvn_str_hash(const mvn_str_t *string_ptr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MVN_DS_STR_H */
