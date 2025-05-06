/*
 * Copyright (c) 2024 Jake Larson
 */
#include "mvn_ds/mvn_ds_str.h"

#include "mvn_ds/mvn_ds_utils.h" // Provides mvn_reallocate, memory macros

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h> // For SIZE_MAX
#include <string.h> // For strlen, memcpy, memcmp

// FNV-1a constants
#define FNV_OFFSET_BASIS 2166136261U
#define FNV_PRIME        16777619U

// --- Static Helper Functions ---

/**
 * @internal
 * @brief Ensures the string has enough capacity for a given additional length.
 * Resizes the string if necessary using MVN_DS_STR_GROWTH_FACTOR.
 * @param string_ptr The string to check/resize. Must not be NULL.
 * @param additional_length The number of additional characters needed (excluding null terminator).
 * @return true if successful (or no resize needed), false on allocation failure or overflow.
 */
static bool mvn_str_ensure_capacity(mvn_str_t *string_ptr, size_t additional_length)
{
    assert(string_ptr != NULL);

    size_t required_length = string_ptr->length + additional_length;

    // Check for potential overflow before comparing with capacity
    if (required_length < string_ptr->length) {
        fprintf(stderr, "[MVN_DS_STR] String length overflow detected.\n");
        return false; // Overflow
    }

    if (required_length <= string_ptr->capacity) {
        return true; // Enough capacity
    }

    // Calculate new capacity
    size_t new_capacity = string_ptr->capacity;
    if (new_capacity == 0) {
        new_capacity = MVN_DS_STR_INITIAL_CAPACITY;
    }
    while (new_capacity < required_length) {
        // Check for overflow before multiplying
        if (new_capacity > SIZE_MAX / MVN_DS_STR_GROWTH_FACTOR) {
            fprintf(stderr, "[MVN_DS_STR] String capacity overflow during growth calculation.\n");
            return false; // Overflow
        }
        new_capacity *= MVN_DS_STR_GROWTH_FACTOR;
    }

    // Check for overflow before adding 1 for null terminator
    if (new_capacity == SIZE_MAX) {
        fprintf(stderr, "[MVN_DS_STR] String capacity reached SIZE_MAX.\n");
        return false; // Cannot add null terminator
    }
    size_t allocation_size = new_capacity + 1; // +1 for null terminator

    char *new_data = (char *)MVN_DS_REALLOC(string_ptr->data, allocation_size);
    if (!new_data) {
        fprintf(stderr, "[MVN_DS_STR] Failed to reallocate string data.\n");
        return false; // Allocation failure
    }

    string_ptr->data     = new_data;
    string_ptr->capacity = new_capacity;
    return true;
}

// --- String Implementation ---

/**
 * @brief Creates a new string with a specific initial capacity.
 * The allocated buffer will be capacity + 1 bytes for the null terminator.
 * The initial string content is empty ("").
 * @param capacity The initial capacity (excluding null terminator).
 * @return A pointer to the new mvn_str_t, or NULL on allocation failure or if capacity is too
 * large.
 */
mvn_str_t *mvn_str_new_capacity(size_t capacity)
{
    /* Copyright (c) 2024 Jake Larson */
    // Prevent capacity + 1 from overflowing or becoming SIZE_MAX.
    // If capacity is SIZE_MAX, capacity + 1 wraps to 0.
    // If capacity is SIZE_MAX - 1, capacity + 1 is SIZE_MAX.
    // Both are problematic: malloc(0) is implementation-defined, malloc(SIZE_MAX) is flagged by
    // Valgrind.
    if (capacity >= SIZE_MAX - 1) { // Check if capacity is SIZE_MAX or SIZE_MAX - 1
        fprintf(stderr,
                "[MVN_DS_STR] Requested capacity %zu is too large or would result in problematic "
                "allocation size.\n",
                capacity);
        return NULL;
    }

    mvn_str_t *string_ptr = (mvn_str_t *)MVN_DS_MALLOC(sizeof(mvn_str_t));
    if (!string_ptr) {
        return NULL; // Malloc failure for the struct itself
    }

    size_t allocation_size = capacity + 1;                           // For null terminator
    string_ptr->data       = (char *)MVN_DS_MALLOC(allocation_size); // This is line 103 from error
    if (!string_ptr->data) {
        MVN_DS_FREE(string_ptr);
        return NULL; // Malloc failure for the data buffer
    }

    string_ptr->length   = 0;
    string_ptr->capacity = capacity;
    string_ptr->data[0]  = '\0'; // Null-terminate the empty string

    return string_ptr;
}

/**
 * @brief Creates a new string by copying a C string.
 * Allocates enough capacity for the copied string, using MVN_DS_STR_INITIAL_CAPACITY
 * as a minimum if the input string is shorter.
 * @param chars The null-terminated C string to copy. If NULL, creates an empty string.
 * @return A pointer to the new mvn_str_t, or NULL on allocation failure.
 */
mvn_str_t *mvn_str_new(const char *chars)
{
    size_t initial_length = (chars == NULL) ? 0 : strlen(chars);
    // Determine initial capacity: at least the length, but use default if larger
    size_t initial_capacity = (initial_length > MVN_DS_STR_INITIAL_CAPACITY) ?
                                  initial_length :
                                  MVN_DS_STR_INITIAL_CAPACITY;

    mvn_str_t *string_ptr = mvn_str_new_capacity(initial_capacity);
    if (!string_ptr) {
        return NULL;
    }

    if (initial_length > 0) {
        memcpy(string_ptr->data, chars, initial_length);
        string_ptr->data[initial_length] = '\0'; // Ensure null termination
        string_ptr->length               = initial_length;
    }
    // Capacity is already set by mvn_str_new_capacity

    return string_ptr;
}

/**
 * @brief Frees the memory associated with a string.
 * Frees the internal data buffer and the string structure itself.
 * @param string_ptr The string to free. Does nothing if NULL.
 */
void mvn_str_free(mvn_str_t *string_ptr)
{
    if (string_ptr == NULL) {
        return;
    }
    MVN_DS_FREE(string_ptr->data); // Free the character buffer
    MVN_DS_FREE(string_ptr);       // Free the struct itself
}

/**
 * @brief Appends a C string to an mvn_str_t.
 * Resizes the string using mvn_str_ensure_capacity if necessary.
 * @param string_ptr The string to append to. Must not be NULL.
 * @param chars The null-terminated C string to append. Must not be NULL.
 * @return true if successful, false on allocation failure or invalid input.
 */
bool mvn_str_append_cstr(mvn_str_t *string_ptr, const char *chars)
{
    if (string_ptr == NULL || chars == NULL) {
        return false;
    }

    size_t append_len = strlen(chars);
    if (append_len == 0) {
        return true; // Nothing to append
    }

    if (!mvn_str_ensure_capacity(string_ptr, append_len)) {
        return false; // Failed to ensure capacity
    }

    // Append the new characters
    memcpy(string_ptr->data + string_ptr->length, chars, append_len);
    string_ptr->length += append_len;
    string_ptr->data[string_ptr->length] = '\0'; // Ensure null termination

    return true;
}

/**
 * @brief Appends another mvn_str_t to an mvn_str_t.
 * Resizes the destination string using mvn_str_ensure_capacity if necessary.
 * @param dest_ptr The destination string. Must not be NULL.
 * @param src_ptr The source string to append. Must not be NULL.
 * @return true if successful, false on allocation failure or invalid input.
 */
bool mvn_str_append(mvn_str_t *dest_ptr, const mvn_str_t *src_ptr)
{
    if (dest_ptr == NULL || src_ptr == NULL) {
        return false;
    }
    if (src_ptr->length == 0) {
        return true; // Nothing to append
    }

    // Use the existing ensure capacity logic
    if (!mvn_str_ensure_capacity(dest_ptr, src_ptr->length)) {
        return false; // Failed to ensure capacity
    }

    // Append the source string's data
    memcpy(dest_ptr->data + dest_ptr->length, src_ptr->data, src_ptr->length);
    dest_ptr->length += src_ptr->length;
    dest_ptr->data[dest_ptr->length] = '\0'; // Ensure null termination

    return true;
}

/**
 * @brief Compares two mvn_str_t strings for equality.
 * Checks if both pointers are non-NULL, have the same length, and the same content.
 * @param str1_ptr The first string.
 * @param str2_ptr The second string.
 * @return true if the strings have the same content, false otherwise. Returns false if either
 * string pointer is NULL.
 */
bool mvn_str_equal(const mvn_str_t *str1_ptr, const mvn_str_t *str2_ptr)
{
    // If either pointer is NULL, they are not equal in the context of valid strings.
    if (str1_ptr == NULL || str2_ptr == NULL) {
        return false;
    }
    // If both point to the exact same object, they are equal.
    if (str1_ptr == str2_ptr) {
        return true;
    }
    // If lengths differ, they are not equal.
    if (str1_ptr->length != str2_ptr->length) {
        return false;
    }
    // If both have length 0, they are equal (empty strings).
    if (str1_ptr->length == 0) {
        return true;
    }
    // Compare content using memcmp.
    return memcmp(str1_ptr->data, str2_ptr->data, str1_ptr->length) == 0;
}

/**
 * @brief Compares an mvn_str_t with a C string for equality.
 * Checks if both pointers are non-NULL, have the same length, and the same content.
 * @param str1_ptr The mvn_str_t.
 * @param cstr2 The null-terminated C string.
 * @return true if the strings have the same content, false otherwise. Returns false if either
 * pointer is NULL.
 */
bool mvn_str_equal_cstr(const mvn_str_t *str1_ptr, const char *cstr2)
{
    // If either pointer is NULL, they are not equal.
    if (str1_ptr == NULL || cstr2 == NULL) {
        return false;
    }

    size_t cstr2_len = strlen(cstr2);
    // If lengths differ, they are not equal.
    if (str1_ptr->length != cstr2_len) {
        return false;
    }
    // If both have length 0, they are equal.
    if (str1_ptr->length == 0) {
        return true;
    }
    // Compare content.
    return memcmp(str1_ptr->data, cstr2, str1_ptr->length) == 0;
}

/**
 * @brief Calculates a hash value for the string (FNV-1a algorithm).
 * Handles NULL string pointers by returning 0.
 * @param string_ptr The string to hash. Can be NULL.
 * @return The 32-bit hash value, or 0 if string_ptr or string_ptr->data is NULL.
 */
uint32_t mvn_str_hash(const mvn_str_t *string_ptr)
{
    if (string_ptr == NULL || string_ptr->data == NULL) {
        return 0; // Return 0 for NULL string or NULL data
    }

    uint32_t hash_value = FNV_OFFSET_BASIS;
    for (size_t index = 0; index < string_ptr->length; ++index) {
        hash_value ^= (uint32_t)string_ptr->data[index];
        hash_value *= FNV_PRIME;
    }
    return hash_value;
}
