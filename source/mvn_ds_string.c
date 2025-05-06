/*
 * Copyright (c) 2024 Jake Larson
 */
#include "mvn_ds/mvn_ds_string.h"

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
 * @brief Ensures the string has enough capacity for a given additional length.
 * Resizes the string if necessary.
 * @param string The string to check/resize. Must not be NULL.
 * @param additional_length The number of additional characters needed (excluding null terminator).
 * @return true if successful (or no resize needed), false on allocation failure.
 */
static bool mvn_string_ensure_capacity(mvn_string_t *string, size_t additional_length)
{
    assert(string != NULL);

    size_t required_length = string->length + additional_length;

    // Check for potential overflow before comparing with capacity
    if (required_length < string->length) {
        fprintf(stderr, "[MVN_DS_STRING] String length overflow detected.\n");
        return false; // Overflow
    }

    if (required_length <= string->capacity) {
        return true; // Enough capacity
    }

    // Calculate new capacity
    size_t new_capacity = string->capacity;
    if (new_capacity == 0) {
        new_capacity = MVN_DS_STRING_INITIAL_CAPACITY;
    }
    while (new_capacity < required_length) {
        // Check for overflow before multiplying
        if (new_capacity > SIZE_MAX / MVN_DS_STRING_GROWTH_FACTOR) {
            fprintf(stderr,
                    "[MVN_DS_STRING] String capacity overflow during growth calculation.\n");
            return false; // Overflow
        }
        new_capacity *= MVN_DS_STRING_GROWTH_FACTOR;
    }

    // Check for overflow before adding 1 for null terminator
    if (new_capacity == SIZE_MAX) {
        fprintf(stderr, "[MVN_DS_STRING] String capacity reached SIZE_MAX.\n");
        return false; // Cannot add null terminator
    }
    size_t allocation_size = new_capacity + 1; // +1 for null terminator

    char *new_data = (char *)MVN_DS_REALLOC(string->data, allocation_size);
    if (!new_data) {
        fprintf(stderr, "[MVN_DS_STRING] Failed to reallocate string data.\n");
        return false; // Allocation failure
    }

    string->data     = new_data;
    string->capacity = new_capacity;
    return true;
}

// --- String Implementation ---

mvn_string_t *mvn_string_new_with_capacity(size_t capacity)
{
    mvn_string_t *string_ptr = (mvn_string_t *)MVN_DS_MALLOC(sizeof(mvn_string_t));
    if (!string_ptr) {
        return NULL;
    }

    string_ptr->length   = 0;
    string_ptr->capacity = capacity;

    // Check for overflow before adding 1 for null terminator
    if (capacity == SIZE_MAX) {
        MVN_DS_FREE(string_ptr);
        fprintf(stderr, "[MVN_DS_STRING] Initial capacity reached SIZE_MAX.\n");
        return NULL;
    }
    size_t allocation_size = capacity + 1; // +1 for null terminator

    string_ptr->data = (char *)MVN_DS_MALLOC(allocation_size);
    if (!string_ptr->data) {
        MVN_DS_FREE(string_ptr);
        return NULL;
    }
    string_ptr->data[0] = '\0'; // Ensure it's always null-terminated

    return string_ptr;
}

mvn_string_t *mvn_string_new(const char *chars)
{
    size_t initial_length = (chars == NULL) ? 0 : strlen(chars);
    // Determine initial capacity: at least the length, but use default if larger
    size_t initial_capacity = (initial_length > MVN_DS_STRING_INITIAL_CAPACITY) ?
                                  initial_length :
                                  MVN_DS_STRING_INITIAL_CAPACITY;

    mvn_string_t *string_ptr = mvn_string_new_with_capacity(initial_capacity);
    if (!string_ptr) {
        return NULL;
    }

    if (initial_length > 0) {
        memcpy(string_ptr->data, chars, initial_length);
        string_ptr->data[initial_length] = '\0'; // Ensure null termination
        string_ptr->length               = initial_length;
    }
    // Capacity is already set by mvn_string_new_with_capacity

    return string_ptr;
}

void mvn_string_free(mvn_string_t *string_ptr)
{
    if (string_ptr == NULL) {
        return;
    }
    MVN_DS_FREE(string_ptr->data); // Free the character buffer
    MVN_DS_FREE(string_ptr);       // Free the struct itself
}

bool mvn_string_append_cstr(mvn_string_t *string_ptr, const char *chars)
{
    if (string_ptr == NULL || chars == NULL) {
        return false;
    }

    size_t append_len = strlen(chars);
    if (append_len == 0) {
        return true; // Nothing to append
    }

    if (!mvn_string_ensure_capacity(string_ptr, append_len)) {
        return false; // Failed to ensure capacity
    }

    // Append the new characters
    memcpy(string_ptr->data + string_ptr->length, chars, append_len);
    string_ptr->length += append_len;
    string_ptr->data[string_ptr->length] = '\0'; // Ensure null termination

    return true;
}

// Implementation for mvn_string_append
bool mvn_string_append(mvn_string_t *dest_ptr, const mvn_string_t *src_ptr)
{
    if (dest_ptr == NULL || src_ptr == NULL) {
        return false;
    }
    if (src_ptr->length == 0) {
        return true; // Nothing to append
    }

    // Use the existing append_cstr logic, passing the source data and length
    if (!mvn_string_ensure_capacity(dest_ptr, src_ptr->length)) {
        return false; // Failed to ensure capacity
    }

    // Append the source string's data
    memcpy(dest_ptr->data + dest_ptr->length, src_ptr->data, src_ptr->length);
    dest_ptr->length += src_ptr->length;
    dest_ptr->data[dest_ptr->length] = '\0'; // Ensure null termination

    return true;
}

bool mvn_string_equal(const mvn_string_t *str1_ptr, const mvn_string_t *str2_ptr)
{
    if (str1_ptr == str2_ptr) {
        return true; // Same pointer or both NULL
    }
    if (str1_ptr == NULL || str2_ptr == NULL) {
        return false; // One is NULL, the other isn't
    }
    if (str1_ptr->length != str2_ptr->length) {
        return false; // Different lengths
    }
    if (str1_ptr->length == 0) {
        return true; // Both are empty strings
    }
    // Compare content using memcmp (safe for potential embedded nulls, though not expected here)
    return memcmp(str1_ptr->data, str2_ptr->data, str1_ptr->length) == 0;
}

// Implementation for mvn_string_equal_cstr
bool mvn_string_equal_cstr(const mvn_string_t *str1_ptr, const char *cstr2)
{
    if (str1_ptr == NULL || cstr2 == NULL) {
        return (str1_ptr == NULL && cstr2 == NULL); // True only if both are NULL (edge case)
                                                    // Standard behavior is false if one is NULL.
                                                    // Let's stick to false if either is NULL.
        // return false; // More conventional: comparison with NULL is false
    }
    if (str1_ptr == NULL || cstr2 == NULL) {
        return false;
    }

    size_t cstr2_len = strlen(cstr2);
    if (str1_ptr->length != cstr2_len) {
        return false; // Different lengths
    }
    if (str1_ptr->length == 0) {
        return true; // Both are empty
    }
    // Compare content
    return memcmp(str1_ptr->data, cstr2, str1_ptr->length) == 0;
}

uint32_t mvn_string_hash(const mvn_string_t *string_ptr)
{
    if (string_ptr == NULL || string_ptr->data == NULL) {
        return 0; // Or some other default hash for NULL
    }

    uint32_t hash_value = FNV_OFFSET_BASIS;
    for (size_t index = 0; index < string_ptr->length; ++index) {
        hash_value ^= (uint32_t)string_ptr->data[index];
        hash_value *= FNV_PRIME;
    }
    return hash_value;
}
