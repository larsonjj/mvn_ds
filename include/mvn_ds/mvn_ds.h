#ifndef MVN_DS_H
#define MVN_DS_H

#include <stdbool.h> // For bool type
#include <stdint.h>  // For int32_t, int64_t etc.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// --- Forward Declarations ---
typedef struct mvn_val_t        mvn_val_t;
typedef struct mvn_string_t     mvn_string_t;
typedef struct mvn_array_t      mvn_array_t;
typedef struct mvn_hmap_t       mvn_hmap_t;
typedef struct mvn_hmap_entry_t mvn_hmap_entry_t;

// --- Type Enum ---
/**
 * @brief Enumeration of possible types stored in mvn_val_t.
 */
typedef enum {
    MVN_VAL_NULL,   /**< Represents a null value. */
    MVN_VAL_BOOL,   /**< Represents a boolean value (true/false). */
    MVN_VAL_I32,    /**< Represents a 32-bit signed integer. */
    MVN_VAL_I64,    /**< Represents a 64-bit signed integer. */
    MVN_VAL_F32,    /**< Represents a 32-bit floating-point number. */
    MVN_VAL_F64,    /**< Represents a 64-bit floating-point number (double). */
    MVN_VAL_STRING, /**< Represents an owned dynamic string (mvn_string_t*). */
    MVN_VAL_ARRAY,  /**< Represents an owned dynamic array (mvn_array_t*). */
    MVN_VAL_HASHMAP /**< Represents an owned dynamic hash map (mvn_hmap_t*). */
} mvn_val_type_t;

// --- Dynamic String ---
/**
 * @brief Structure representing a dynamic, null-terminated string.
 */
struct mvn_string_t {
    size_t length;   /**< Current length of the string (excluding null terminator). */
    size_t capacity; /**< Allocated capacity of the character buffer. */
    char  *data;     /**< Pointer to the null-terminated character buffer. */
};

// --- Dynamic Array ---
/**
 * @brief Structure representing a dynamic array of mvn_val_t values.
 */
struct mvn_array_t {
    size_t     count;    /**< Number of elements currently in the array. */
    size_t     capacity; /**< Allocated capacity of the value buffer. */
    mvn_val_t *data;     /**< Pointer to the buffer holding mvn_val_t elements. */
};

// --- Generic Value ---
/**
 * @brief A tagged union structure capable of holding various data types.
 * Owns the memory for MVN_VAL_STRING, MVN_VAL_ARRAY, and MVN_VAL_HASHMAP types.
 */
struct mvn_val_t {
    mvn_val_type_t type;    /**< The type of data currently held by the union. */
    union {                 // Anonymous union
        bool          b;    /**< Value if type is MVN_VAL_BOOL. */
        int32_t       i32;  /**< Value if type is MVN_VAL_I32. */
        int64_t       i64;  /**< Value if type is MVN_VAL_I64. */
        float         f32;  /**< Value if type is MVN_VAL_F32. */
        double        f64;  /**< Value if type is MVN_VAL_F64. */
        mvn_string_t *str;  /**< Pointer to owned string if type is MVN_VAL_STRING. */
        mvn_array_t  *arr;  /**< Pointer to owned array if type is MVN_VAL_ARRAY. */
        mvn_hmap_t   *hmap; /**< Pointer to owned hash map if type is MVN_VAL_HASHMAP. */
    };
};

// --- Hash Map Entry ---
/**
 * @brief Structure representing a single key-value entry in a hash map.
 * Used internally for chaining.
 */
struct mvn_hmap_entry_t {
    mvn_string_t     *key;   /**< Owned key for the entry. */
    mvn_val_t         value; /**< Owned value for the entry. */
    mvn_hmap_entry_t *next;  /**< Pointer to the next entry in case of collision
                                (separate chaining). */
};

// --- Hash Map ---
/**
 * @brief Structure representing a dynamic hash map with string keys and
 * mvn_val_t values.
 */
struct mvn_hmap_t {
    size_t             count;    /**< Number of key-value pairs currently in the map. */
    size_t             capacity; /**< Number of buckets allocated. */
    mvn_hmap_entry_t **buckets;  /**< Pointer to the array of bucket pointers
                                    (heads of linked lists). */
};

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
 * @param value Pointer to the value to free.
 */
void mvn_val_free(mvn_val_t *value);

/**
 * @brief Prints a representation of the value to stdout (for debugging).
 * @param value Pointer to the value to print.
 */
void mvn_val_print(const mvn_val_t *value);

// --- String Operations ---
mvn_string_t *mvn_string_new(const char *chars);
mvn_string_t *mvn_string_new_with_capacity(size_t capacity);
void          mvn_string_free(mvn_string_t *string);
bool          mvn_string_append_cstr(mvn_string_t *string, const char *chars);
bool          mvn_string_equal(const mvn_string_t *s1, const mvn_string_t *s2);
uint32_t      mvn_string_hash(const mvn_string_t *string); // Hash function

// --- Array Operations ---
mvn_array_t *mvn_array_new(void);
mvn_array_t *mvn_array_new_with_capacity(size_t capacity);
void         mvn_array_free(mvn_array_t *array);
/**
 * @brief Appends a value to the end of the array.
 * The array takes ownership of the value if it's a dynamic type (STRING, ARRAY,
 * HASHMAP).
 * @param array The array to append to.
 * @param value The value to append.
 * @return true if successful, false on allocation failure.
 */
bool       mvn_array_push(mvn_array_t *array, mvn_val_t value);
mvn_val_t *mvn_array_get(const mvn_array_t *array, size_t index);
/**
 * @brief Sets the value at a specific index in the array.
 * Frees the existing value at the index before setting the new one.
 * The array takes ownership of the new value if it's a dynamic type.
 * @param array The array to modify.
 * @param index The index to set.
 * @param value The new value.
 * @return true if successful (index was valid), false otherwise.
 */
bool mvn_array_set(mvn_array_t *array, size_t index, mvn_val_t value);

// --- Hash Map Operations ---
mvn_hmap_t *mvn_hmap_new(void);
mvn_hmap_t *mvn_hmap_new_with_capacity(size_t capacity);
void        mvn_hmap_free(mvn_hmap_t *hmap);
/**
 * @brief Sets a key-value pair in the hash map.
 * Takes ownership of the key string and the value's dynamic data.
 * Frees the existing value if the key already exists. Frees the *provided* key
 * if the key already exists (as the existing key is kept).
 * @param hmap The hash map.
 * @param key The key (ownership is taken).
 * @param value The value (ownership is taken if dynamic).
 * @return true if successful, false on allocation failure.
 */
bool mvn_hmap_set(mvn_hmap_t *hmap, mvn_string_t *key, mvn_val_t value);
/**
 * @brief Sets a key-value pair using a C string for the key.
 * Creates a new mvn_string_t for the key internally and takes ownership.
 * Takes ownership of the value's dynamic data.
 * Frees the existing value if the key already exists.
 * @param hmap The hash map.
 * @param key_cstr The C string key.
 * @param value The value (ownership is taken if dynamic).
 * @return true if successful, false on allocation failure.
 */
bool       mvn_hmap_set_cstr(mvn_hmap_t *hmap, const char *key_cstr, mvn_val_t value);
mvn_val_t *mvn_hmap_get(const mvn_hmap_t *hmap, const mvn_string_t *key);
mvn_val_t *mvn_hmap_get_cstr(const mvn_hmap_t *hmap, const char *key_cstr);
/**
 * @brief Deletes a key-value pair from the hash map.
 * Frees the key string and the associated value.
 * @param hmap The hash map.
 * @param key The key to delete.
 * @return true if the key was found and deleted, false otherwise.
 */
bool mvn_hmap_delete(mvn_hmap_t *hmap, const mvn_string_t *key);
/**
 * @brief Deletes a key-value pair using a C string key.
 * Frees the key string and the associated value.
 * @param hmap The hash map.
 * @param key_cstr The C string key to delete.
 * @return true if the key was found and deleted, false otherwise.
 */
bool mvn_hmap_delete_cstr(mvn_hmap_t *hmap, const char *key_cstr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // MVN_DS_H
