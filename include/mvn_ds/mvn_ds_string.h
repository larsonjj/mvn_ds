/*
 * Copyright (c) 2024 Jake Larson
 */
#ifndef MVN_DS_STRING_H
#define MVN_DS_STRING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// --- Dynamic String ---
/**
 * @brief Structure representing a dynamic, null-terminated string.
 */
typedef struct mvn_string_t {
    size_t length;   /**< Current length of the string (excluding null terminator). */
    size_t capacity; /**< Allocated capacity of the character buffer. */
    char  *data;     /**< Pointer to the null-terminated character buffer. */
} mvn_string_t;

// --- String Operations ---

/**
 * @brief Creates a new dynamic string by copying a C string.
 * @param chars The null-terminated C string to copy.
 * @return A pointer to the new mvn_string_t, or NULL on allocation failure.
 */
mvn_string_t *mvn_string_new(const char *chars);

/**
 * @brief Creates a new dynamic string with a specific initial capacity.
 * The string is initially empty.
 * @param capacity The initial capacity (excluding null terminator).
 * @return A pointer to the new mvn_string_t, or NULL on allocation failure.
 */
mvn_string_t *mvn_string_new_with_capacity(size_t capacity);

/**
 * @brief Frees the memory associated with a dynamic string.
 * @param string The string to free. Does nothing if NULL.
 */
void mvn_string_free(mvn_string_t *string);

/**
 * @brief Appends a C string to the end of a dynamic string.
 * Resizes the dynamic string if necessary.
 * @param string The dynamic string to append to. Must not be NULL.
 * @param chars The null-terminated C string to append. Must not be NULL.
 * @return true if successful, false on allocation failure or invalid input.
 */
bool mvn_string_append_cstr(mvn_string_t *string, const char *chars);

/**
 * @brief Compares two dynamic strings for equality.
 * @param str_one The first string.
 * @param str_two The second string.
 * @return true if the strings have the same content, false otherwise. Also
 * returns false if either string is NULL.
 */
bool mvn_string_equal(const mvn_string_t *str_one, const mvn_string_t *str_two);

/**
 * @brief Calculates a hash value for a dynamic string (FNV-1a algorithm).
 * @param string The string to hash.
 * @return A 32-bit hash value. Returns 0 if the string or its data is NULL.
 */
uint32_t mvn_string_hash(const mvn_string_t *string);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MVN_DS_STRING_H */
