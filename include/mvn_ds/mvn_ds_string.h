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

// --- Dynamic String ---
// struct mvn_string_t is now defined in mvn_ds_types.h

// --- String Operations ---

/**
 * @brief Creates a new string by copying a C string.
 * @param chars The null-terminated C string to copy. If NULL, creates an empty string.
 * @return A pointer to the new mvn_string_t, or NULL on allocation failure.
 */
mvn_string_t *mvn_string_new(const char *chars);

/**
 * @brief Creates a new string with a specific initial capacity.
 * @param capacity The initial capacity (excluding null terminator).
 * @return A pointer to the new mvn_string_t, or NULL on allocation failure.
 */
mvn_string_t *mvn_string_new_with_capacity(size_t capacity);

/**
 * @brief Frees the memory associated with a string.
 * @param string The string to free. Does nothing if NULL.
 */
void mvn_string_free(mvn_string_t *string);

/**
 * @brief Appends a C string to an mvn_string_t.
 * Resizes the string if necessary.
 * @param string The string to append to. Must not be NULL.
 * @param chars The null-terminated C string to append. Must not be NULL.
 * @return true if successful, false on allocation failure.
 */
bool mvn_string_append_cstr(mvn_string_t *string, const char *chars);

/**
 * @brief Appends another mvn_string_t to an mvn_string_t.
 * Resizes the string if necessary.
 * @param dest The destination string. Must not be NULL.
 * @param src The source string to append. Must not be NULL.
 * @return true if successful, false on allocation failure.
 */
bool mvn_string_append(mvn_string_t *dest, const mvn_string_t *src);

/**
 * @brief Compares two mvn_string_t strings for equality.
 * @param str1 The first string.
 * @param str2 The second string.
 * @return true if the strings have the same content, false otherwise. Returns false if either
 * string is NULL.
 */
bool mvn_string_equal(const mvn_string_t *str1, const mvn_string_t *str2);

/**
 * @brief Compares an mvn_string_t with a C string for equality.
 * @param str1 The mvn_string_t.
 * @param cstr2 The null-terminated C string.
 * @return true if the strings have the same content, false otherwise. Returns false if either is
 * NULL.
 */
bool mvn_string_equal_cstr(const mvn_string_t *str1, const char *cstr2);

/**
 * @brief Calculates a hash value for the string (FNV-1a algorithm).
 * @param string The string to hash. Must not be NULL.
 * @return The 32-bit hash value.
 */
uint32_t mvn_string_hash(const mvn_string_t *string);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MVN_DS_STRING_H */
