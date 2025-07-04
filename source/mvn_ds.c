/*
 * Copyright (c) 2024 Jake Larson
 */
#include "mvn_ds/mvn_ds.h"

#include "mvn_ds/mvn_ds_arr.h"
#include "mvn_ds/mvn_ds_hmap.h"
#include "mvn_ds/mvn_ds_str.h"
#include "mvn_ds/mvn_ds_utils.h" // For MVN_DS_MALLOC, MVN_DS_FREE

#include <ctype.h>    // For isprint
#include <inttypes.h> // For PRId64, PRIu32, PRIu64
#include <math.h>     // For fabs, fabsf
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h> // For int32_t, int64_t etc.
#include <stdio.h>  // For printf (used in mvn_val_print)
#include <stdlib.h> // For SIZE_MAX (if any remain)
#include <string.h> // For strlen (if any remain), and memcpy for mvn_val_deep_copy

// Define epsilon values for floating-point comparisons
#define MVN_DS_FLOAT_EPSILON  1e-6f
#define MVN_DS_DOUBLE_EPSILON 1e-14

// --- Value Implementation ---

/**
 * @brief Creates a NULL value.
 * @return A mvn_val_t representing NULL.
 */
mvn_val_t mvn_val_null(void)
{
    return (mvn_val_t){.type = MVN_VAL_NULL};
}

/**
 * @brief Creates a boolean value.
 * @param b The boolean value.
 * @return A mvn_val_t representing the boolean.
 */
mvn_val_t mvn_val_bool(bool b_val)
{
    return (mvn_val_t){.type = MVN_VAL_BOOL, .b = b_val};
}

/**
 * @brief Creates an 8-bit integer value.
 * @param i8_val The integer value.
 * @return A mvn_val_t representing the integer.
 */
mvn_val_t mvn_val_i8(int8_t i8_val)
{
    mvn_val_t val_item;
    val_item.type = MVN_VAL_I8;
    val_item.i8   = i8_val;
    return val_item;
}

/**
 * @brief Creates a 16-bit integer value.
 * @param i16_val The integer value.
 * @return A mvn_val_t representing the integer.
 */
mvn_val_t mvn_val_i16(int16_t i16_val)
{
    mvn_val_t val_item;
    val_item.type = MVN_VAL_I16;
    val_item.i16  = i16_val;
    return val_item;
}

/**
 * @brief Creates a 32-bit integer value.
 * @param i32 The integer value.
 * @return A mvn_val_t representing the integer.
 */
mvn_val_t mvn_val_i32(int32_t i32)
{
    return (mvn_val_t){.type = MVN_VAL_I32, .i32 = i32};
}

/**
 * @brief Creates a 64-bit integer value.
 * @param i64 The integer value.
 * @return A mvn_val_t representing the integer.
 */
mvn_val_t mvn_val_i64(int64_t i64_val)
{
    mvn_val_t val_item;
    val_item.type = MVN_VAL_I64;
    val_item.i64  = i64_val;
    return val_item;
}

/**
 * @brief Creates an 8-bit unsigned integer value.
 * @param u8_val The unsigned integer value.
 * @return A mvn_val_t representing the unsigned integer.
 */
mvn_val_t mvn_val_u8(uint8_t u8_val)
{
    mvn_val_t val_item;
    val_item.type = MVN_VAL_U8;
    val_item.u8   = u8_val;
    return val_item;
}

/**
 * @brief Creates a 16-bit unsigned integer value.
 * @param u16_val The unsigned integer value.
 * @return A mvn_val_t representing the unsigned integer.
 */
mvn_val_t mvn_val_u16(uint16_t u16_val)
{
    mvn_val_t val_item;
    val_item.type = MVN_VAL_U16;
    val_item.u16  = u16_val;
    return val_item;
}

/**
 * @brief Creates a 32-bit unsigned integer value.
 * @param u32 The unsigned integer value.
 * @return A mvn_val_t representing the unsigned integer.
 */
mvn_val_t mvn_val_u32(uint32_t u32_val)
{
    mvn_val_t val_item;
    val_item.type = MVN_VAL_U32;
    val_item.u32  = u32_val;
    return val_item;
}

/**
 * @brief Creates a 64-bit unsigned integer value.
 * @param u64 The unsigned integer value.
 * @return A mvn_val_t representing the unsigned integer.
 */
mvn_val_t mvn_val_u64(uint64_t u64_val)
{
    mvn_val_t val_item;
    val_item.type = MVN_VAL_U64;
    val_item.u64  = u64_val;
    return val_item;
}

/**
 * @brief Creates a 32-bit float value.
 * @param f32 The float value.
 * @return A mvn_val_t representing the float.
 */
mvn_val_t mvn_val_f32(float f32_val)
{
    mvn_val_t val_item;
    val_item.type = MVN_VAL_F32;
    val_item.f32  = f32_val;
    return val_item;
}

/**
 * @brief Creates a 64-bit double value.
 * @param f64 The double value.
 * @return A mvn_val_t representing the double.
 */
mvn_val_t mvn_val_f64(double f64_val)
{
    mvn_val_t val_item;
    val_item.type = MVN_VAL_F64;
    val_item.f64  = f64_val;
    return val_item;
}

/**
 * @brief Creates a character value.
 * @param c The character value.
 * @return A mvn_val_t representing the character.
 */
mvn_val_t mvn_val_char(char char_val)
{
    mvn_val_t val_item;
    val_item.type = MVN_VAL_CHAR;
    val_item.c    = char_val;
    return val_item;
}

/**
 * @brief Creates a generic pointer value.
 * @param ptr_val The pointer value.
 * @return A mvn_val_t representing the pointer.
 * @note The mvn_val_t does NOT take ownership of the pointed-to memory.
 *       The caller is responsible for managing the lifetime of the data pointed to by ptr_val.
 */
mvn_val_t mvn_val_ptr(void *ptr_val)
{
    mvn_val_t val_item;
    val_item.type = MVN_VAL_PTR;
    val_item.ptr  = ptr_val;
    return val_item;
}

/**
 * @brief Creates a string value by copying a C string.
 * Allocates a new mvn_str_t internally.
 * @param chars The C string to copy. If NULL, creates an empty string value.
 * @return A mvn_val_t representing the string, or MVN_VAL_NULL on allocation failure.
 */
mvn_val_t mvn_val_str(const char *chars)
{
    mvn_str_t *str = mvn_str_new(chars);
    if (!str) {
        return mvn_val_null(); // Handle allocation failure
    }
    return (mvn_val_t){.type = MVN_VAL_STRING, .str = str};
}

/**
 * @brief Creates a string value by taking ownership of an existing mvn_str_t.
 * The provided string pointer will be managed by the mvn_val_t.
 * @param str The mvn_str_t to take ownership of. If NULL, creates a NULL value.
 * @return A mvn_val_t representing the string.
 */
mvn_val_t mvn_val_str_take(mvn_str_t *str)
{
    if (!str) {
        return mvn_val_null();
    }
    return (mvn_val_t){.type = MVN_VAL_STRING, .str = str};
}

/**
 * @brief Creates an empty array value.
 * Allocates a new mvn_arr_t internally.
 * @return A mvn_val_t representing the array, or MVN_VAL_NULL on allocation failure.
 */
mvn_val_t mvn_val_arr(void)
{
    mvn_arr_t *arr = mvn_arr_new();
    if (!arr) {
        return mvn_val_null(); // Handle allocation failure
    }
    return (mvn_val_t){.type = MVN_VAL_ARRAY, .arr = arr};
}

/**
 * @brief Creates an array value by taking ownership of an existing mvn_arr_t.
 * The provided array pointer will be managed by the mvn_val_t.
 * @param arr The mvn_arr_t to take ownership of. If NULL, creates a NULL value.
 * @return A mvn_val_t representing the array.
 */
mvn_val_t mvn_val_arr_take(mvn_arr_t *arr)
{
    if (!arr) {
        return mvn_val_null();
    }
    return (mvn_val_t){.type = MVN_VAL_ARRAY, .arr = arr};
}

/**
 * @brief Creates an empty hash map value.
 * Allocates a new mvn_hmap_t internally.
 * @return A mvn_val_t representing the hash map, or MVN_VAL_NULL on allocation failure.
 */
mvn_val_t mvn_val_hmap(void)
{
    mvn_hmap_t *hmap = mvn_hmap_new(); // Calls function now defined in mvn_ds_hmap.c
    if (!hmap) {
        return mvn_val_null(); // Handle allocation failure
    }
    return (mvn_val_t){.type = MVN_VAL_HASHMAP, .hmap = hmap};
}

/**
 * @brief Creates a hash map value by taking ownership of an existing mvn_hmap_t.
 * The provided map pointer will be managed by the mvn_val_t.
 * @param hmap The mvn_hmap_t to take ownership of. If NULL, creates a NULL value.
 * @return A mvn_val_t representing the hash map.
 */
mvn_val_t mvn_val_hmap_take(mvn_hmap_t *hmap)
{
    if (!hmap) {
        return mvn_val_null();
    }
    return (mvn_val_t){.type = MVN_VAL_HASHMAP, .hmap = hmap};
}

/**
 * @brief Frees the resources owned by a mvn_val_t.
 * If the value type is STRING, ARRAY, or HASHMAP, it frees the associated
 * dynamic structure recursively. For other types, it does nothing.
 * Resets the value to MVN_VAL_NULL after freeing to prevent double frees.
 * @param value Pointer to the value to free. Does nothing if NULL.
 */
void mvn_val_free(mvn_val_t *value)
{
    if (!value) {
        return;
    }
    switch (value->type) {
            // Dynamic types that need freeing:
        case MVN_VAL_STRING:
            mvn_str_free(value->str); // Calls function from mvn_ds_str.c
            break;
        case MVN_VAL_ARRAY:
            mvn_arr_free(value->arr); // Calls function from mvn_ds_arr.c
            break;
        case MVN_VAL_HASHMAP:
            mvn_hmap_free(value->hmap); // Calls function from mvn_ds_hmap.c
            break;
            // Primitive types and NULL don't own heap resources:
        case MVN_VAL_NULL:
        case MVN_VAL_BOOL:
        case MVN_VAL_I8:
        case MVN_VAL_I16:
        case MVN_VAL_I32:
        case MVN_VAL_I64:
        case MVN_VAL_U8:
        case MVN_VAL_U16:
        case MVN_VAL_U32:
        case MVN_VAL_U64:
        case MVN_VAL_F32:
        case MVN_VAL_F64:
        case MVN_VAL_CHAR:
        case MVN_VAL_PTR:
            // No action needed
            break;
            // Default case for safety, although all enum values should be handled
        default:
            fprintf(
                stderr, "[MVN_DS] Warning: mvn_val_free called on unknown type %d\n", value->type);
            break;
    }
    // Reset to NULL to prevent double frees and indicate it's no longer valid
    *value = mvn_val_null();
}

/**
 * @brief Converts a mvn_val_type_t enum to its string representation.
 * @param type The mvn_val_type_t value.
 * @return A string literal representing the type, or "UNKNOWN" for invalid types.
 */
const char *mvn_val_type_to_str(mvn_val_type_t type)
{
    switch (type) {
        case MVN_VAL_NULL:
            return "NULL";
        case MVN_VAL_BOOL:
            return "BOOL";
        case MVN_VAL_I8:
            return "I8";
        case MVN_VAL_I16:
            return "I16";
        case MVN_VAL_I32:
            return "I32";
        case MVN_VAL_I64:
            return "I64";
        case MVN_VAL_U8:
            return "U8";
        case MVN_VAL_U16:
            return "U16";
        case MVN_VAL_U32:
            return "U32";
        case MVN_VAL_U64:
            return "U64";
        case MVN_VAL_F32:
            return "F32";
        case MVN_VAL_F64:
            return "F64";
        case MVN_VAL_CHAR:
            return "CHAR";
        case MVN_VAL_PTR:
            return "PTR";
        case MVN_VAL_STRING:
            return "STRING";
        case MVN_VAL_ARRAY:
            return "ARRAY";
        case MVN_VAL_HASHMAP:
            return "HASHMAP";
        default:
            return "UNKNOWN";
    }
}

/**
 * @brief Prints a representation of the value to stdout (for debugging).
 * Follows a JSON-like format. Handles NULL pointers gracefully.
 * @param value Pointer to the value to print.
 */
void mvn_val_print(const mvn_val_t *value)
{
    if (!value) {
        printf("NULL_VALUE_PTR");
        return;
    }
    switch (value->type) {
        case MVN_VAL_NULL:
            printf("null");
            break;
        case MVN_VAL_BOOL:
            printf("Bool(%s)", value->b ? "true" : "false");
            break;
        case MVN_VAL_I8:
            printf("I8(%" PRId8 ")", value->i8);
            break;
        case MVN_VAL_I16:
            printf("I16(%" PRId16 ")", value->i16);
            break;
        case MVN_VAL_I32:
            printf("%" PRId32, value->i32); // Use PRI macros for portability
            break;
        case MVN_VAL_I64:
            printf("I64(%" PRId64 ")", value->i64);
            break;
        case MVN_VAL_U8:
            printf("U8(%" PRIu8 ")", value->u8);
            break;
        case MVN_VAL_U16:
            printf("U16(%" PRIu16 ")", value->u16);
            break;
        case MVN_VAL_U32:
            printf("U32(%" PRIu32 ")", value->u32);
            break;
        case MVN_VAL_U64:
            printf("U64(%" PRIu64 ")", value->u64);
            break;
        case MVN_VAL_F32:
            printf("F32(%f)", (double)value->f32); // Promote to double for printf
            break;
        case MVN_VAL_F64:
            printf("F64(%f)", value->f64);
            break;
        case MVN_VAL_CHAR:
            if (isprint(value->c)) {
                printf("Char('%c')", value->c);
            } else {
                printf("Char(0x%02X)", (unsigned char)value->c);
            }
            break;
        case MVN_VAL_PTR:
            printf("Ptr(%p)", value->ptr);
            break;
        case MVN_VAL_STRING:
            // Check str and str->data for validity
            printf("\"%s\"", (value->str && value->str->data) ? value->str->data : "NULL_STR");
            break;
        case MVN_VAL_ARRAY:
            if (!value->arr) {
                printf("NULL_ARR_PTR");
                break;
            }
            printf("[");
            for (size_t index = 0; index < value->arr->count; index++) {
                // Check data pointer before accessing element
                if (value->arr->data) {
                    mvn_val_print(&value->arr->data[index]);
                } else {
                    printf("INVALID_ARR_DATA"); // Should not happen if count > 0
                }

                if (index < value->arr->count - 1) {
                    printf(", ");
                }
            }
            printf("]");
            break;
        case MVN_VAL_HASHMAP: { // Use braces for scope
            if (!value->hmap) {
                printf("NULL_HMAP_PTR");
                break;
            }
            printf("{");
            bool first = true;
            // Check if buckets exist before iterating
            if (value->hmap->buckets) {
                for (size_t index = 0; index < value->hmap->capacity; index++) {
                    mvn_hmap_entry_t *entry = value->hmap->buckets[index];
                    while (entry) {
                        if (!first) {
                            printf(", ");
                        }
                        first = false;
                        // Assume key is always valid if entry exists, check data
                        printf("\"%s\": ",
                               (entry->key && entry->key->data) ? entry->key->data : "NULL_KEY");
                        mvn_val_print(&entry->value);
                        entry = entry->next;
                    }
                }
            }
            printf("}");
            break;
        } // Close brace for case
        default:
            printf("UNKNOWN_TYPE(%d)", value->type);
            break;
    }
}

/**
 * @brief Compares two mvn_val_t values for equality.
 * Handles different types and performs deep comparison for dynamic types.
 * For floating-point types (F32, F64), uses a small epsilon for comparison.
 * @param val_one Pointer to the first value.
 * @param val_two Pointer to the second value.
 * @return true if the values are considered equal, false otherwise.
 */
bool mvn_val_equal(const mvn_val_t *val_one, const mvn_val_t *val_two)
{
    // Handle NULL pointers: two NULL pointers are equal, one NULL is not.
    if (!val_one && !val_two) {
        return true;
    }
    if (!val_one || !val_two) {
        return false;
    }

    // If types differ, they are not equal.
    if (val_one->type != val_two->type) {
        return false;
    }

    // Compare based on type
    switch (val_one->type) {
        case MVN_VAL_NULL:
            return true; // NULL is always equal to NULL
        case MVN_VAL_BOOL:
            return val_one->b == val_two->b;
        case MVN_VAL_I8:
            return val_one->i8 == val_two->i8;
        case MVN_VAL_I16:
            return val_one->i16 == val_two->i16;
        case MVN_VAL_I32:
            return val_one->i32 == val_two->i32;
        case MVN_VAL_I64:
            return val_one->i64 == val_two->i64;
        case MVN_VAL_U8:
            return val_one->u8 == val_two->u8;
        case MVN_VAL_U16:
            return val_one->u16 == val_two->u16;
        case MVN_VAL_U32:
            return val_one->u32 == val_two->u32;
        case MVN_VAL_U64:
            return val_one->u64 == val_two->u64;
        case MVN_VAL_F32:
            // Use a small epsilon for float comparison
            return fabsf(val_one->f32 - val_two->f32) < MVN_DS_FLOAT_EPSILON;
        case MVN_VAL_F64:
            // Use a small epsilon for double comparison
            return fabs(val_one->f64 - val_two->f64) < MVN_DS_DOUBLE_EPSILON;
        case MVN_VAL_CHAR:
            return val_one->c == val_two->c;
        case MVN_VAL_PTR:
            return val_one->ptr == val_two->ptr;
        case MVN_VAL_STRING:
            // Use mvn_str_equal, handles NULL internal data pointers
            return mvn_str_equal(val_one->str, val_two->str);
        case MVN_VAL_ARRAY: {
            mvn_arr_t *arr_one = val_one->arr;
            mvn_arr_t *arr_two = val_two->arr;

            // Handle NULL array pointers within the value
            if (!arr_one && !arr_two) {
                return true;
            }
            if (!arr_one || !arr_two) {
                return false;
            }
            if (arr_one->count != arr_two->count) {
                return false;
            }
            // Check data pointers before iterating
            if (!arr_one->data && !arr_two->data && arr_one->count == 0) {
                return true; // Both empty and potentially NULL data
            }
            if (!arr_one->data || !arr_two->data) {
                return false; // One has data, the other doesn't (and count > 0)
            }

            // Recursively compare elements
            for (size_t index = 0; index < arr_one->count; index++) {
                if (!mvn_val_equal(&arr_one->data[index], &arr_two->data[index])) {
                    return false;
                }
            }
            return true;
        }
        case MVN_VAL_HASHMAP: {
            mvn_hmap_t *map_one = val_one->hmap;
            mvn_hmap_t *map_two = val_two->hmap;

            // Handle NULL map pointers within the value
            if (!map_one && !map_two) {
                return true;
            }
            if (!map_one || !map_two) {
                return false;
            }
            if (map_one->count != map_two->count) {
                return false;
            }
            if (map_one->count == 0) {
                return true; // Both are empty
            }
            // Check buckets pointers before iterating
            if (!map_one->buckets || !map_two->buckets) {
                // This case should ideally not happen if count > 0, but check defensively
                return false;
            }

            // Iterate through the first map and check against the second
            for (size_t index = 0; index < map_one->capacity; index++) {
                mvn_hmap_entry_t *entry_one = map_one->buckets[index];
                while (entry_one) {
                    // Find the corresponding key in the second map
                    mvn_val_t *found_val_two = mvn_hmap_get(map_two, entry_one->key);
                    if (!found_val_two) {
                        return false; // Key not found in the second map
                    }
                    // Recursively compare the values
                    if (!mvn_val_equal(&entry_one->value, found_val_two)) {
                        return false; // Values for the same key are different
                    }
                    entry_one = entry_one->next;
                }
            }
            return true; // All keys and values matched
        }
        default:
            // Should not happen if all types are handled
            fprintf(stderr,
                    "[MVN_DS] Warning: mvn_val_equal called on unknown type %d\n",
                    val_one->type);
            return false;
    }
}

/**
 * @brief Creates a deep copy of a mvn_val_t.
 * For dynamic types (STRING, ARRAY, HASHMAP), this means new allocations and copying content.
 * For PTR type, the pointer value is copied, not the data it points to.
 * Primitive types are copied by value.
 * @param original_value Pointer to the value to copy.
 * @return A new mvn_val_t containing the deep-copied data.
 *         Returns MVN_VAL_NULL if original_value is NULL or on allocation failure.
 */
mvn_val_t mvn_val_deep_copy(const mvn_val_t *original_value)
{
    if (!original_value) {
        return mvn_val_null();
    }

    mvn_val_t copy_val;
    copy_val.type = original_value->type;

    switch (original_value->type) {
        case MVN_VAL_NULL:
            // Handled by mvn_val_null() if needed, but type is already set
            break;
        case MVN_VAL_BOOL:
            copy_val.b = original_value->b;
            break;
        case MVN_VAL_I8:
            copy_val.i8 = original_value->i8;
            break;
        case MVN_VAL_I16:
            copy_val.i16 = original_value->i16;
            break;
        case MVN_VAL_I32:
            copy_val.i32 = original_value->i32;
            break;
        case MVN_VAL_I64:
            copy_val.i64 = original_value->i64;
            break;
        case MVN_VAL_U8:
            copy_val.u8 = original_value->u8;
            break;
        case MVN_VAL_U16:
            copy_val.u16 = original_value->u16;
            break;
        case MVN_VAL_U32:
            copy_val.u32 = original_value->u32;
            break;
        case MVN_VAL_U64:
            copy_val.u64 = original_value->u64;
            break;
        case MVN_VAL_F32:
            copy_val.f32 = original_value->f32;
            break;
        case MVN_VAL_F64:
            copy_val.f64 = original_value->f64;
            break;
        case MVN_VAL_CHAR:
            copy_val.c = original_value->c;
            break;
        case MVN_VAL_PTR:
            copy_val.ptr = original_value->ptr; // Shallow copy of pointer
            break;
        case MVN_VAL_STRING:
            if (original_value->str && original_value->str->data) {
                // Create a new string and copy content
                mvn_str_t *new_str_ptr = mvn_str_new(original_value->str->data);
                if (!new_str_ptr) {
                    return mvn_val_null(); // Allocation failure
                }
                copy_val.str = new_str_ptr;
            } else {
                // Original string was NULL or had NULL data, create an empty or NULL string value
                copy_val.str = mvn_str_new(""); // Or handle as mvn_val_null() if preferred
                if (!copy_val.str) {
                    return mvn_val_null();
                }
            }
            break;
        case MVN_VAL_ARRAY:
            if (original_value->arr) {
                mvn_arr_t *new_arr_ptr = mvn_arr_new_capacity(original_value->arr->count);
                if (!new_arr_ptr) {
                    return mvn_val_null(); // Allocation failure
                }
                for (size_t i = 0; i < original_value->arr->count; ++i) {
                    mvn_val_t element_copy = mvn_val_deep_copy(&original_value->arr->data[i]);
                    if (!mvn_arr_push(new_arr_ptr, element_copy)) {
                        // Handle push failure: free already copied elements and the new array
                        mvn_val_free(&element_copy); // Free the last problematic copy
                        mvn_arr_free(new_arr_ptr);   // Frees other elements pushed so far
                        return mvn_val_null();
                    }
                }
                copy_val.arr = new_arr_ptr;
            } else {
                copy_val.arr = NULL; // Or mvn_arr_new() for an empty array
            }
            break;
        case MVN_VAL_HASHMAP:
            if (original_value->hmap) {
                mvn_hmap_t *new_hmap_ptr = mvn_hmap_new_capacity(original_value->hmap->capacity);
                if (!new_hmap_ptr) {
                    return mvn_val_null(); // Allocation failure
                }
                // Iterate through the original hash map
                // This requires an iterator or access to buckets, which mvn_hmap_t provides
                for (size_t i = 0; i < original_value->hmap->capacity; ++i) {
                    mvn_hmap_entry_t *current_entry = original_value->hmap->buckets[i];
                    while (current_entry) {
                        if (current_entry->key) { // Ensure key is not NULL
                            mvn_str_t *key_copy_ptr = mvn_str_new(current_entry->key->data);
                            if (!key_copy_ptr) {
                                mvn_hmap_free(new_hmap_ptr);
                                return mvn_val_null();
                            }
                            mvn_val_t value_copy = mvn_val_deep_copy(&current_entry->value);

                            if (!mvn_hmap_set(new_hmap_ptr, key_copy_ptr, value_copy)) {
                                // Handle set failure
                                mvn_str_free(key_copy_ptr);
                                mvn_val_free(&value_copy);
                                mvn_hmap_free(new_hmap_ptr);
                                return mvn_val_null();
                            }
                        }
                        current_entry = current_entry->next;
                    }
                }
                copy_val.hmap = new_hmap_ptr;
            } else {
                copy_val.hmap = NULL; // Or mvn_hmap_new() for an empty map
            }
            break;
        default:
            // Should not happen if all types are handled
            fprintf(stderr,
                    "[MVN_DS] Warning: mvn_val_deep_copy called on unknown type %d\n",
                    original_value->type);
            return mvn_val_null();
    }
    return copy_val;
}

/**
 * @brief Compares two mvn_val_t values.
 *
 * Defines a total order for mvn_val_t instances.
 * - First compares by type (e.g., NULL < BOOL < I32 < STRING).
 * - If types are the same, performs type-specific comparison.
 * - For strings, uses lexicographical comparison.
 * - For arrays and hashmaps, comparison is currently based on pointer address or count
 *   (could be extended to content comparison if needed, but that's complex).
 *   For simplicity in this example, we'll compare by count, then by pointer if counts are equal.
 *
 * @param val_one The first value.
 * @param val_two The second value.
 * @return <0 if val_one < val_two, 0 if equal, >0 if val_one > val_two.
 *         Returns 0 if both are NULL pointers.
 *         If one is NULL and other is not, NULL is considered less.
 */
int mvn_val_compare(const mvn_val_t *val_one, const mvn_val_t *val_two)
{
    if (val_one == NULL && val_two == NULL) return 0;
    if (val_one == NULL) return -1; // NULL is less than non-NULL
    if (val_two == NULL) return 1;  // Non-NULL is greater than NULL

    if (val_one->type < val_two->type) return -1;
    if (val_one->type > val_two->type) return 1;

    // Types are the same, perform type-specific comparison
    switch (val_one->type) {
        case MVN_VAL_NULL:
            return 0; // Both are NULL type
        case MVN_VAL_BOOL:
            return (val_one->b == val_two->b) ? 0 : (val_one->b < val_two->b ? -1 : 1);
        case MVN_VAL_I8:
            return (val_one->i8 == val_two->i8) ? 0 : (val_one->i8 < val_two->i8 ? -1 : 1);
        case MVN_VAL_I16:
            return (val_one->i16 == val_two->i16) ? 0 : (val_one->i16 < val_two->i16 ? -1 : 1);
        case MVN_VAL_I32:
            return (val_one->i32 == val_two->i32) ? 0 : (val_one->i32 < val_two->i32 ? -1 : 1);
        case MVN_VAL_I64:
            return (val_one->i64 == val_two->i64) ? 0 : (val_one->i64 < val_two->i64 ? -1 : 1);
        case MVN_VAL_U8:
            return (val_one->u8 == val_two->u8) ? 0 : (val_one->u8 < val_two->u8 ? -1 : 1);
        case MVN_VAL_U16:
            return (val_one->u16 == val_two->u16) ? 0 : (val_one->u16 < val_two->u16 ? -1 : 1);
        case MVN_VAL_U32:
            return (val_one->u32 == val_two->u32) ? 0 : (val_one->u32 < val_two->u32 ? -1 : 1);
        case MVN_VAL_U64:
            return (val_one->u64 == val_two->u64) ? 0 : (val_one->u64 < val_two->u64 ? -1 : 1);
        case MVN_VAL_F32: {
            float diff = val_one->f32 - val_two->f32;
            if (fabsf(diff) < MVN_DS_FLOAT_EPSILON) return 0;
            return diff < 0 ? -1 : 1;
        }
        case MVN_VAL_F64: {
            double diff = val_one->f64 - val_two->f64;
            if (fabs(diff) < MVN_DS_DOUBLE_EPSILON) return 0;
            return diff < 0 ? -1 : 1;
        }
        case MVN_VAL_CHAR:
            return (val_one->c == val_two->c) ? 0 : (val_one->c < val_two->c ? -1 : 1);
        case MVN_VAL_PTR: // Compare by pointer address
            if (val_one->ptr == val_two->ptr) return 0;
            return (val_one->ptr < val_two->ptr) ? -1 : 1;
        case MVN_VAL_STRING:
            if (val_one->str == val_two->str) return 0; // Both point to same string or both NULL
            if (!val_one->str) return -1;               // NULL string is less
            if (!val_two->str) return 1;                // Non-NULL string is greater
            // Both strings are non-NULL, compare their data
            if (val_one->str->data == val_two->str->data) return 0;
            if (!val_one->str->data) return -1;
            if (!val_two->str->data) return 1;
            return strcmp(val_one->str->data, val_two->str->data);
        case MVN_VAL_ARRAY:
            // Simplified comparison: by count, then by address.
            if (val_one->arr == val_two->arr) return 0;
            if (!val_one->arr) return -1;
            if (!val_two->arr) return 1;
            if (val_one->arr->count < val_two->arr->count) return -1;
            if (val_one->arr->count > val_two->arr->count) return 1;
            // Counts are equal, compare by address for a consistent order
            return (val_one->arr < val_two->arr) ? -1 : (val_one->arr > val_two->arr ? 1 : 0);
        case MVN_VAL_HASHMAP:
            // Simplified comparison: by count, then by address.
            if (val_one->hmap == val_two->hmap) return 0;
            if (!val_one->hmap) return -1;
            if (!val_two->hmap) return 1;
            if (val_one->hmap->count < val_two->hmap->count) return -1;
            if (val_one->hmap->count > val_two->hmap->count) return 1;
            return (val_one->hmap < val_two->hmap) ? -1 : (val_one->hmap > val_two->hmap ? 1 : 0);
        default:
            return 0; // Should not happen
    }
}
