/*
 * Copyright (c) 2024 Jake Larson
 */
#ifndef MVN_DS_TYPES_H
#define MVN_DS_TYPES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// --- Forward Declarations ---
// Forward declare all structs first to handle interdependencies
typedef struct mvn_string_t     mvn_string_t;
typedef struct mvn_array_t      mvn_array_t;
typedef struct mvn_hmap_entry_t mvn_hmap_entry_t;
typedef struct mvn_hmap_t       mvn_hmap_t;
typedef struct mvn_val_t        mvn_val_t;

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
    size_t capacity; /**< Allocated capacity of the data buffer (excluding null terminator). */
    char  *data;     /**< Pointer to the character buffer. Always null-terminated. */
};

// --- Generic Value ---
// Define mvn_val_t before types that embed it directly (like mvn_hmap_entry_t)
// or use it in arrays (like mvn_array_t).
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

// --- Dynamic Array ---
/**
 * @brief Structure representing a dynamic array of mvn_val_t values.
 */
struct mvn_array_t {
    size_t     count;    /**< Number of elements currently in the array. */
    size_t     capacity; /**< Allocated capacity of the value buffer. */
    mvn_val_t *data;     /**< Pointer to the buffer holding mvn_val_t elements. */
};

// --- Hash Map Entry ---
// Now mvn_val_t is fully defined.
/**
 * @brief Structure representing a single key-value entry in a hash map.
 * Used internally for chaining.
 */
struct mvn_hmap_entry_t {
    mvn_string_t     *key;   /**< Owned key for the entry. */
    mvn_val_t         value; /**< Owned value for the entry. */
    mvn_hmap_entry_t *next;  /**< Pointer to the next entry in case of collision. */
};

// --- Hash Map ---
// Now mvn_hmap_entry_t is fully defined.
/**
 * @brief Structure representing a dynamic hash map with string keys and
 * mvn_val_t values. Uses separate chaining for collision resolution.
 */
struct mvn_hmap_t {
    size_t             count;    /**< Number of key-value pairs currently in the map. */
    size_t             capacity; /**< Number of buckets allocated. */
    mvn_hmap_entry_t **buckets;  /**< Pointer to the array of bucket pointers. */
};

#endif /* MVN_DS_TYPES_H */
