/*
 * Copyright (c) 2024 Jake Larson
 */
#include "mvn_ds/mvn_ds_string.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Defines ---
// NOTE: These might be better placed in a shared internal header
#define MVN_INITIAL_CAPACITY 8
#define MVN_GROWTH_FACTOR    2

// --- Memory Allocation Aliases ---
// NOTE: Using stdlib directly here. Replace with SDL_* if required globally.
#define MVN_MALLOC  malloc
#define MVN_CALLOC  calloc
#define MVN_REALLOC realloc
#define MVN_FREE    free

// --- Internal Helper Functions ---

/**
 * @internal
 * @brief Reallocates memory, handling potential errors.
 * Exits on failure for simplicity in this example. Robust applications
 * should handle this more gracefully. Uses MVN_REALLOC and MVN_FREE.
 * @param pointer Existing pointer (or NULL).
 * @param old_size Current allocated size (ignored by standard realloc but
 * useful for custom allocators).
 * @param new_size Desired new size. If 0, frees the pointer.
 * @return Pointer to the reallocated memory, or NULL if new_size is 0 or allocation fails.
 */
static void *mvn_reallocate(void *pointer, size_t old_size, size_t new_size)
{
    (void)old_size; // Unused in this basic implementation
    if (new_size == 0) {
        MVN_FREE(pointer);
        return NULL;
    }
    void *result = MVN_REALLOC(pointer, new_size);
    if (result == NULL && new_size > 0) { // Check if allocation actually failed
        fprintf(stderr, "[MVN_DS_STRING] Memory allocation failed!\n");
        // In a real library, you might return NULL and let the caller handle it.
        // For now, mimic previous behavior but return NULL
        // exit(EXIT_FAILURE);
    }
    return result;
}

/**
 * @internal
 * @brief Ensures string has enough capacity, reallocating if necessary.
 * @param string The string to check/grow.
 * @param additional_length The minimum additional length needed.
 * @return true if successful (or no resize needed), false on allocation
 * failure.
 */
static bool mvn_string_ensure_capacity(mvn_string_t *string, size_t additional_length)
{
    assert(string != NULL);
    // Check for potential overflow before calculating required capacity
    if (SIZE_MAX - additional_length < string->length) {
        fprintf(stderr, "[MVN_DS_STRING] String length overflow during capacity check.\n");
        return false;
    }
    size_t required_capacity = string->length + additional_length;

    if (required_capacity < string->capacity) {
        return true; // Enough space
    }

    size_t old_capacity = string->capacity;
    size_t new_capacity = old_capacity < MVN_INITIAL_CAPACITY ? MVN_INITIAL_CAPACITY :
                                                                old_capacity * MVN_GROWTH_FACTOR;
    // Ensure the new capacity is at least what's required
    while (new_capacity <= required_capacity) {
        // Check for potential overflow during growth calculation
        if (SIZE_MAX / MVN_GROWTH_FACTOR < new_capacity) {
            fprintf(stderr,
                    "[MVN_DS_STRING] String capacity overflow during resize calculation.\n");
            // Try setting to exactly required capacity if growth calculation overflows
            if (required_capacity > old_capacity) {
                new_capacity = required_capacity;
                break; // Exit loop, try allocating required_capacity
            } else {
                return false; // Cannot even allocate required capacity
            }
        }
        new_capacity *= MVN_GROWTH_FACTOR;
    }

    // +1 for null terminator
    if (SIZE_MAX - 1 < new_capacity) {
        fprintf(stderr, "[MVN_DS_STRING] String capacity overflow adding null terminator space.\n");
        return false;
    }
    size_t allocation_size = new_capacity + 1;

    char *new_data = (char *)mvn_reallocate(string->data, old_capacity + 1, allocation_size);
    if (!new_data) {
        return false; // Reallocation failed
    }

    string->data     = new_data;
    string->capacity = new_capacity;
    return true;
}

// --- String Implementation ---

mvn_string_t *mvn_string_new_with_capacity(size_t capacity)
{
    mvn_string_t *string = (mvn_string_t *)MVN_MALLOC(sizeof(mvn_string_t));
    if (!string) {
        return NULL;
    }

    string->length   = 0;
    string->capacity = capacity;
    // +1 for null terminator
    if (SIZE_MAX - 1 < capacity) { // Check overflow before adding 1
        MVN_FREE(string);
        return NULL;
    }
    string->data = (char *)MVN_MALLOC(capacity + 1);
    if (!string->data) {
        MVN_FREE(string);
        return NULL;
    }
    string->data[0] = '\0'; // Ensure null termination for empty string
    return string;
}

mvn_string_t *mvn_string_new(const char *chars)
{
    if (!chars) {
        return NULL; // Handle null input gracefully
    }
    size_t length = strlen(chars);
    // Start with at least initial capacity or required length
    size_t        initial_capacity = length < MVN_INITIAL_CAPACITY ? MVN_INITIAL_CAPACITY : length;
    mvn_string_t *string           = mvn_string_new_with_capacity(initial_capacity);
    if (!string) {
        return NULL;
    }

    memcpy(string->data, chars, length);
    string->data[length] = '\0';
    string->length       = length;
    return string;
}

void mvn_string_free(mvn_string_t *string)
{
    if (!string) {
        return;
    }
    MVN_FREE(string->data); // Free the character buffer
    MVN_FREE(string);       // Free the struct itself
}

bool mvn_string_append_cstr(mvn_string_t *string, const char *chars)
{
    if (!string || !chars) {
        return false;
    }
    size_t append_len = strlen(chars);
    if (append_len == 0) {
        return true; // Nothing to append
    }

    if (!mvn_string_ensure_capacity(string, append_len)) {
        return false; // Failed to resize
    }

    memcpy(string->data + string->length, chars, append_len);
    string->length += append_len;
    string->data[string->length] = '\0'; // Ensure null termination
    return true;
}

bool mvn_string_equal(const mvn_string_t *str_one, const mvn_string_t *str_two)
{
    // If one is NULL and the other isn't, they are not equal.
    if (!str_one || !str_two) {
        // If both are NULL, consider them unequal as per previous test logic.
        // If only one is NULL, they are unequal.
        return false;
    }
    // If they point to the exact same memory (and are not NULL), they are equal.
    if (str_one == str_two) {
        return true;
    }
    // Both are non-NULL and different pointers, compare content.
    // Check length first for quick exit
    if (str_one->length != str_two->length) {
        return false;
    }
    // If length is 0, they are equal (already checked lengths are same)
    if (str_one->length == 0) {
        return true;
    }
    // Compare data content
    return memcmp(str_one->data, str_two->data, str_one->length) == 0;
}

uint32_t mvn_string_hash(const mvn_string_t *string)
{
    if (!string || !string->data) {
        return 0; // Handle NULL string or data
    }

    uint32_t hash_value = 2166136261u; // FNV offset basis
    for (size_t index = 0; index < string->length; index++) {
        hash_value ^= (uint8_t)string->data[index];
        hash_value *= 16777619; // FNV prime
    }
    return hash_value;
}
