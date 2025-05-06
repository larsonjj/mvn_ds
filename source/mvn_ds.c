#include "mvn_ds/mvn_ds.h"

#include <assert.h>   // For basic assertions
#include <inttypes.h> // For PRI macros like PRId64
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h> // For int32_t, int64_t etc.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define MVN_INITIAL_CAPACITY 8
#define MVN_GROWTH_FACTOR    2
#define MVN_MAP_LOAD_FACTOR  0.75

// --- Memory Allocation Helper ---

/**
 * @internal
 * @brief Reallocates memory, handling potential errors.
 * Exits on failure for simplicity in this example. Robust applications
 * should handle this more gracefully.
 * @param pointer Existing pointer (or NULL).
 * @param old_size Current allocated size (ignored by standard realloc but
 * useful for custom allocators).
 * @param new_size Desired new size. If 0, frees the pointer.
 * @return Pointer to the reallocated memory, or NULL if new_size is 0.
 */
static void *mvn_reallocate(void *pointer, size_t old_size, size_t new_size)
{
    (void)old_size; // Unused in this basic implementation
    if (new_size == 0) {
        free(pointer);
        return NULL;
    }
    void *result = realloc(pointer, new_size);
    if (result == NULL) {
        fprintf(stderr, "[MVN_DS] Memory allocation failed!\n");
        return NULL;
    }
    return result;
}

// --- String Implementation ---

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
    if (string->length + additional_length < string->capacity) {
        return true; // Enough space
    }

    size_t old_capacity = string->capacity;
    size_t new_capacity = old_capacity < MVN_INITIAL_CAPACITY ? MVN_INITIAL_CAPACITY :
                                                                old_capacity * MVN_GROWTH_FACTOR;
    while (new_capacity <= string->length + additional_length) {
        new_capacity *= MVN_GROWTH_FACTOR;
        // Prevent potential overflow if capacity grows excessively large
        if (new_capacity < old_capacity) {
            fprintf(stderr, "[MVN_DS] String capacity overflow.\n");
            return false; // Indicate failure
        }
    }

    char *new_data = (char *)mvn_reallocate(string->data, old_capacity + 1, new_capacity + 1);
    if (!new_data)
        return false; // Reallocation failed

    string->data     = new_data;
    string->capacity = new_capacity;
    return true;
}

/**
 * @brief Creates a new dynamic string with a specific initial capacity.
 * @param capacity The initial capacity (excluding null terminator).
 * @return A pointer to the new mvn_string_t, or NULL on allocation failure.
 */
mvn_string_t *mvn_string_new_with_capacity(size_t capacity)
{
    mvn_string_t *string = (mvn_string_t *)malloc(sizeof(mvn_string_t));
    if (!string)
        return NULL;

    string->length   = 0;
    string->capacity = capacity;
    string->data     = (char *)malloc(capacity + 1); // +1 for null terminator
    if (!string->data) {
        free(string);
        return NULL;
    }
    string->data[0] = '\0'; // Ensure null termination for empty string
    return string;
}

/**
 * @brief Creates a new dynamic string by copying a C string.
 * @param chars The null-terminated C string to copy.
 * @return A pointer to the new mvn_string_t, or NULL on allocation failure.
 */
mvn_string_t *mvn_string_new(const char *chars)
{
    if (!chars)
        return NULL; // Handle null input gracefully
    size_t length = strlen(chars);
    // Start with at least initial capacity or required length
    size_t        initial_capacity = length < MVN_INITIAL_CAPACITY ? MVN_INITIAL_CAPACITY : length;
    mvn_string_t *string           = mvn_string_new_with_capacity(initial_capacity);
    if (!string)
        return NULL;

    memcpy(string->data, chars, length);
    string->data[length] = '\0';
    string->length       = length;
    return string;
}

/**
 * @brief Frees the memory associated with a dynamic string.
 * @param string The string to free. Does nothing if NULL.
 */
void mvn_string_free(mvn_string_t *string)
{
    if (!string)
        return;
    free(string->data); // Free the character buffer
    free(string);       // Free the struct itself
}

/**
 * @brief Appends a C string to the end of a dynamic string.
 * @param string The dynamic string to append to.
 * @param chars The null-terminated C string to append.
 * @return true if successful, false on allocation failure.
 */
bool mvn_string_append_cstr(mvn_string_t *string, const char *chars)
{
    if (!string || !chars)
        return false;
    size_t append_len = strlen(chars);
    if (append_len == 0)
        return true; // Nothing to append

    if (!mvn_string_ensure_capacity(string, append_len)) {
        return false; // Failed to resize
    }

    memcpy(string->data + string->length, chars, append_len);
    string->length += append_len;
    string->data[string->length] = '\0'; // Ensure null termination
    return true;
}

/**
 * @brief Compares two dynamic strings for equality.
 * @param s1 The first string.
 * @param s2 The second string.
 * @return true if the strings have the same content, false otherwise. Also
 * returns false if either string is NULL.
 */
bool mvn_string_equal(const mvn_string_t *s1, const mvn_string_t *s2)
{
    if (s1 == s2)
        return true; // Same pointer
    if (!s1 || !s2)
        return false; // One or both are NULL
    // Check length first for quick exit
    return s1->length == s2->length && memcmp(s1->data, s2->data, s1->length) == 0;
}

/**
 * @brief Calculates a hash value for a dynamic string (FNV-1a algorithm).
 * @param string The string to hash.
 * @return A 32-bit hash value. Returns 0 if the string is NULL.
 */
uint32_t mvn_string_hash(const mvn_string_t *string)
{
    if (!string || !string->data)
        return 0; // Handle NULL string or data

    uint32_t hash = 2166136261u; // FNV offset basis
    for (size_t i = 0; i < string->length; i++) {
        hash ^= (uint8_t)string->data[i];
        hash *= 16777619; // FNV prime
    }
    return hash;
}

// --- Array Implementation ---

/**
 * @internal
 * @brief Ensures array has enough capacity, reallocating if necessary.
 * @param array The array to check/grow.
 * @return true if successful (or no resize needed), false on allocation
 * failure.
 */
static bool mvn_array_ensure_capacity(mvn_array_t *array)
{
    if (array->count < array->capacity) {
        return true; // Enough space
    }
    size_t old_capacity = array->capacity;
    size_t new_capacity = old_capacity < MVN_INITIAL_CAPACITY ? MVN_INITIAL_CAPACITY :
                                                                old_capacity * MVN_GROWTH_FACTOR;

    // Prevent potential overflow
    if (new_capacity < old_capacity) {
        fprintf(stderr, "[MVN_DS] Array capacity overflow.\n");
        return false;
    }

    mvn_val_t *new_data = (mvn_val_t *)mvn_reallocate(
        array->data, old_capacity * sizeof(mvn_val_t), new_capacity * sizeof(mvn_val_t));
    if (!new_data)
        return false;

    array->data     = new_data;
    array->capacity = new_capacity;

    // Initialize new slots to NULL to prevent freeing uninitialized memory later
    for (size_t i = old_capacity; i < new_capacity; ++i) {
        array->data[i] = mvn_val_null();
    }
    return true;
}

/**
 * @brief Creates a new dynamic array with a specific initial capacity.
 * @param capacity The initial capacity.
 * @return Pointer to the new array, or NULL on allocation failure.
 */
mvn_array_t *mvn_array_new_with_capacity(size_t capacity)
{
    mvn_array_t *array = (mvn_array_t *)malloc(sizeof(mvn_array_t));
    if (!array)
        return NULL;

    array->count    = 0;
    array->capacity = capacity;
    if (capacity > 0) {
        array->data = (mvn_val_t *)malloc(capacity * sizeof(mvn_val_t));
        if (!array->data) {
            free(array);
            return NULL;
        }
        // Initialize to NULL
        for (size_t i = 0; i < capacity; ++i) {
            array->data[i] = mvn_val_null();
        }
    } else {
        array->data = NULL; // No initial allocation if capacity is 0
    }
    return array;
}

/**
 * @brief Creates a new dynamic array with a default initial capacity.
 * @return Pointer to the new array, or NULL on allocation failure.
 */
mvn_array_t *mvn_array_new(void)
{
    return mvn_array_new_with_capacity(MVN_INITIAL_CAPACITY);
}

/**
 * @brief Frees the memory associated with a dynamic array, including its
 * elements. Calls mvn_val_free on each element before freeing the array itself.
 * @param array The array to free. Does nothing if NULL.
 */
void mvn_array_free(mvn_array_t *array)
{
    if (!array)
        return;
    // Free contained values first
    for (size_t i = 0; i < array->count; i++) {
        mvn_val_free(&array->data[i]);
    }
    free(array->data); // Free the value buffer
    free(array);       // Free the struct itself
}

/**
 * @brief Appends a value to the end of the array.
 * The array takes ownership of the value if it's a dynamic type (STRING, ARRAY,
 * HASHMAP).
 * @param array The array to append to.
 * @param value The value to append.
 * @return true if successful, false on allocation failure.
 */
bool mvn_array_push(mvn_array_t *array, mvn_val_t value)
{
    if (!array)
        return false;
    if (!mvn_array_ensure_capacity(array)) {
        // If resize failed, we might own the value now but can't store it.
        // Free it to prevent leaks.
        mvn_val_free(&value);
        return false;
    }
    // Place the value (transfers ownership)
    array->data[array->count++] = value;
    return true;
}

/**
 * @brief Gets a pointer to the value at a specific index.
 * @param array The array to access.
 * @param index The index of the element.
 * @return Pointer to the value at the index, or NULL if the index is out of
 * bounds or array is NULL.
 */
mvn_val_t *mvn_array_get(const mvn_array_t *array, size_t index)
{
    if (!array || index >= array->count) {
        return NULL;
    }
    // Const cast is safe here if the user respects the const-ness of the return
    // when calling via a const array*. If they have a non-const array*, they get
    // a non-const value* back.
    return &((mvn_array_t *)array)->data[index];
}

/**
 * @brief Sets the value at a specific index in the array.
 * Frees the existing value at the index before setting the new one.
 * The array takes ownership of the new value if it's a dynamic type.
 * @param array The array to modify.
 * @param index The index to set.
 * @param value The new value.
 * @return true if successful (index was valid), false otherwise.
 */
bool mvn_array_set(mvn_array_t *array, size_t index, mvn_val_t value)
{
    if (!array || index >= array->count) {
        // Index out of bounds. Free the incoming value as it won't be stored.
        mvn_val_free(&value);
        return false;
    }
    // Free the old value at the index before overwriting
    mvn_val_free(&array->data[index]);
    // Assign the new value (transfers ownership)
    array->data[index] = value;
    return true;
}

// --- Hash Map Implementation ---

/**
 * @internal
 * @brief Finds the map entry for a given key.
 * @param buckets The bucket array.
 * @param capacity The capacity of the bucket array.
 * @param key The key to search for.
 * @param hash The precomputed hash of the key.
 * @return Pointer to the entry if found, NULL otherwise.
 */
static mvn_hmap_entry_t *mvn_hmap_find_entry(mvn_hmap_entry_t  **buckets,
                                             size_t              capacity,
                                             const mvn_string_t *key,
                                             uint32_t            hash)
{
    if (capacity == 0)
        return NULL;
    size_t            index = hash % capacity; // Simple modulo distribution
    mvn_hmap_entry_t *entry = buckets[index];

    // Traverse the linked list (chain) at this bucket index
    while (entry != NULL) {
        // Check hash first (quick check), then full string equality
        if (mvn_string_hash(entry->key) == hash && mvn_string_equal(entry->key, key)) {
            return entry; // Found it
        }
        entry = entry->next;
    }
    return NULL; // Not found
}

/**
 * @internal
 * @brief Resizes the hash map's bucket array and rehashes entries.
 * @param hmap The hash map to resize.
 * @param new_capacity The desired new capacity.
 */
static void mvn_hmap_adjust_capacity(mvn_hmap_t *hmap, size_t new_capacity)
{
    if (new_capacity == 0) {
        // Cannot resize to zero capacity if there are elements
        assert(hmap->count == 0);
        // Free old buckets if they exist
        free(hmap->buckets);
        hmap->buckets  = NULL;
        hmap->capacity = 0;
        return;
    }

    // Allocate new bucket array, initialized to NULL
    mvn_hmap_entry_t **new_buckets =
        (mvn_hmap_entry_t **)calloc(new_capacity, sizeof(mvn_hmap_entry_t *));
    if (!new_buckets) {
        // Allocation failed, keep the old table. This is problematic.
        // A real implementation might try other strategies or report error.
        fprintf(stderr, "[MVN_DS] Hash map resize failed - out of memory.\n");
        // Keep old table, performance will degrade.
        return;
    }

    // Rehash all existing entries into the new buckets
    hmap->count = 0; // Reset count, will be incremented as items are added back
    for (size_t i = 0; i < hmap->capacity; i++) {
        mvn_hmap_entry_t *entry = hmap->buckets[i];
        while (entry != NULL) {
            mvn_hmap_entry_t *next = entry->next; // Store next entry before modifying current

            // Recalculate index in the new bucket array
            uint32_t hash  = mvn_string_hash(entry->key);
            size_t   index = hash % new_capacity;

            // Insert entry at the head of the new bucket's list
            entry->next        = new_buckets[index];
            new_buckets[index] = entry;
            hmap->count++; // Increment count as we re-insert

            entry = next; // Move to the next entry in the old chain
        }
    }

    free(hmap->buckets); // Free the old bucket array
    hmap->buckets  = new_buckets;
    hmap->capacity = new_capacity;
}

/**
 * @brief Creates a new hash map with a specific initial capacity.
 * @param capacity The initial number of buckets.
 * @return Pointer to the new map, or NULL on allocation failure.
 */
mvn_hmap_t *mvn_hmap_new_with_capacity(size_t capacity)
{
    mvn_hmap_t *hmap = (mvn_hmap_t *)malloc(sizeof(mvn_hmap_t));
    if (!hmap)
        return NULL;

    hmap->count    = 0;
    hmap->capacity = capacity;
    if (capacity > 0) {
        // Use calloc to initialize bucket pointers to NULL
        hmap->buckets = (mvn_hmap_entry_t **)calloc(capacity, sizeof(mvn_hmap_entry_t *));
        if (!hmap->buckets) {
            free(hmap);
            return NULL;
        }
    } else {
        hmap->buckets = NULL;
    }
    return hmap;
}

/**
 * @brief Creates a new hash map with a default initial capacity.
 * @return Pointer to the new map, or NULL on allocation failure.
 */
mvn_hmap_t *mvn_hmap_new(void)
{
    return mvn_hmap_new_with_capacity(MVN_INITIAL_CAPACITY);
}

/**
 * @brief Frees the memory associated with a hash map.
 * Frees all keys, values (recursively), entries, the bucket array, and the map
 * struct.
 * @param hmap The hash map to free. Does nothing if NULL.
 */
void mvn_hmap_free(mvn_hmap_t *hmap)
{
    if (!hmap)
        return;
    for (size_t i = 0; i < hmap->capacity; i++) {
        mvn_hmap_entry_t *entry = hmap->buckets[i];
        while (entry != NULL) {
            mvn_hmap_entry_t *next = entry->next; // Store next pointer
            mvn_string_free(entry->key);          // Free the key string
            mvn_val_free(&entry->value);          // Free the value (recursively)
            free(entry);                          // Free the entry struct
            entry = next;                         // Move to the next entry
        }
    }
    free(hmap->buckets); // Free the bucket array
    free(hmap);          // Free the map struct
}

/**
 * @brief Sets a key-value pair in the hash map.
 * Takes ownership of the key string and the value's dynamic data.
 * Frees the existing value if the key already exists. Frees the *provided* key
 * if the key already exists (as the existing key is kept).
 * @param hmap The hash map.
 * @param key The key (ownership is taken).
 * @param value The value (ownership is taken if dynamic).
 * @return true if successful, false on allocation failure or if key is NULL.
 */
bool mvn_hmap_set(mvn_hmap_t *hmap, mvn_string_t *key, mvn_val_t value)
{
    if (!hmap || !key) {
        // Invalid input. Free potentially owned value and provided key.
        mvn_val_free(&value);
        mvn_string_free(key); // Safe even if key is NULL
        return false;
    }

    // Resize if load factor exceeds threshold
    if (hmap->count + 1 > hmap->capacity * MVN_MAP_LOAD_FACTOR) {
        size_t new_capacity = hmap->capacity < MVN_INITIAL_CAPACITY ?
                                  MVN_INITIAL_CAPACITY :
                                  hmap->capacity * MVN_GROWTH_FACTOR;
        // Check for potential overflow during growth calculation
        if (new_capacity < hmap->capacity) {
            fprintf(stderr, "[MVN_DS] Hash map capacity overflow during resize calculation.\n");
            // Free key/value as we cannot insert
            mvn_string_free(key);
            mvn_val_free(&value);
            return false;
        }
        mvn_hmap_adjust_capacity(hmap, new_capacity);
        // Note: adjust_capacity might fail internally, but we proceed assuming it
        // worked or kept the old table.
    }

    uint32_t           hash  = mvn_string_hash(key);
    size_t             index = hash % hmap->capacity;
    mvn_hmap_entry_t **bucket_ptr =
        &hmap->buckets[index]; // Pointer to the slot holding the head pointer

    // Traverse chain to check if key exists
    while (*bucket_ptr != NULL) {
        mvn_hmap_entry_t *entry = *bucket_ptr;
        // Check hash first, then full equality
        if (mvn_string_hash(entry->key) == hash && mvn_string_equal(entry->key, key)) {
            // Key found - replace value
            mvn_val_free(&entry->value); // Free the old value
            entry->value = value;        // Assign the new value (takes ownership)
            mvn_string_free(key);        // Free the *provided* key, as we keep the existing one
            return true;
        }
        bucket_ptr = &entry->next; // Move to the next pointer in the chain
    }

    // Key not found - create new entry
    mvn_hmap_entry_t *new_entry = (mvn_hmap_entry_t *)malloc(sizeof(mvn_hmap_entry_t));
    if (!new_entry) {
        // Allocation failed - free the key and value we were given
        mvn_string_free(key);
        mvn_val_free(&value);
        return false;
    }

    new_entry->key   = key;   // Takes ownership of the provided key
    new_entry->value = value; // Takes ownership of the provided value
    new_entry->next  = NULL;  // It's the new end of the chain

    // Link the new entry into the bucket (at the end of the chain where
    // bucket_ptr points)
    *bucket_ptr = new_entry;
    hmap->count++;
    return true;
}

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
bool mvn_hmap_set_cstr(mvn_hmap_t *hmap, const char *key_cstr, mvn_val_t value)
{
    if (!key_cstr) {
        mvn_val_free(&value); // Free value if key is invalid
        return false;
    }
    mvn_string_t *key_string = mvn_string_new(key_cstr);
    if (!key_string) {
        mvn_val_free(&value); // Free value if key string creation fails
        return false;
    }
    // mvn_hmap_set takes ownership of key_string and value
    if (!mvn_hmap_set(hmap, key_string, value)) {
        // If set fails, it should have already freed key_string and value
        return false;
    }
    return true;
}

/**
 * @brief Gets a pointer to the value associated with a specific key.
 * @param hmap The hash map.
 * @param key The key to search for.
 * @return Pointer to the value if the key is found, NULL otherwise.
 */
mvn_val_t *mvn_hmap_get(const mvn_hmap_t *hmap, const mvn_string_t *key)
{
    if (!hmap || !key || hmap->capacity == 0) {
        return NULL;
    }
    uint32_t          hash  = mvn_string_hash(key);
    mvn_hmap_entry_t *entry = mvn_hmap_find_entry(hmap->buckets, hmap->capacity, key, hash);
    return entry ? &entry->value : NULL;
}

/**
 * @brief Gets a pointer to the value associated with a C string key.
 * @param hmap The hash map.
 * @param key_cstr The C string key to search for.
 * @return Pointer to the value if the key is found, NULL otherwise.
 */
mvn_val_t *mvn_hmap_get_cstr(const mvn_hmap_t *hmap, const char *key_cstr)
{
    if (!hmap || !key_cstr || hmap->capacity == 0) {
        return NULL;
    }
    // Create a temporary string wrapper for searching, without allocating new
    // memory for the chars. This is slightly risky if the lifetime of key_cstr is
    // shorter than the map lookup, but common for this type of function. Be
    // cautious if key_cstr is volatile. The cast discards const but
    // mvn_string_hash/equal won't modify it.
    mvn_string_t temp_key = {.length = strlen(key_cstr), .capacity = 0, .data = (char *)key_cstr};
    uint32_t     hash     = mvn_string_hash(&temp_key);

    // Need to cast away const from hmap temporarily to call non-const find_entry
    // This is safe as find_entry doesn't modify the map structure itself.
    mvn_hmap_entry_t *entry =
        mvn_hmap_find_entry(((mvn_hmap_t *)hmap)->buckets, hmap->capacity, &temp_key, hash);
    return entry ? &entry->value : NULL;
}

/**
 * @brief Deletes a key-value pair from the hash map.
 * Frees the key string and the associated value.
 * @param hmap The hash map.
 * @param key The key to delete.
 * @return true if the key was found and deleted, false otherwise.
 */
bool mvn_hmap_delete(mvn_hmap_t *hmap, const mvn_string_t *key)
{
    if (!hmap || !key || hmap->capacity == 0) {
        return false;
    }

    uint32_t           hash       = mvn_string_hash(key);
    size_t             index      = hash % hmap->capacity;
    mvn_hmap_entry_t **bucket_ptr = &hmap->buckets[index]; // Pointer to the link pointing to the
                                                           // current entry
    mvn_hmap_entry_t *entry = *bucket_ptr;

    while (entry != NULL) {
        // Check hash and then equality
        if (mvn_string_hash(entry->key) == hash && mvn_string_equal(entry->key, key)) {
            // Found the entry, unlink it
            *bucket_ptr = entry->next; // Make the previous link point to the next entry

            // Free the deleted entry's contents and the entry itself
            mvn_string_free(entry->key);
            mvn_val_free(&entry->value);
            free(entry);
            hmap->count--;
            return true; // Successfully deleted
        }
        // Move to the next entry's link
        bucket_ptr = &entry->next;
        entry      = *bucket_ptr;
    }

    return false; // Key not found
}

/**
 * @brief Deletes a key-value pair using a C string key.
 * Frees the key string and the associated value.
 * @param hmap The hash map.
 * @param key_cstr The C string key to delete.
 * @return true if the key was found and deleted, false otherwise.
 */
bool mvn_hmap_delete_cstr(mvn_hmap_t *hmap, const char *key_cstr)
{
    if (!hmap || !key_cstr || hmap->capacity == 0) {
        return false;
    }
    // Use the temporary string trick again for lookup
    mvn_string_t temp_key = {.length = strlen(key_cstr), .capacity = 0, .data = (char *)key_cstr};
    // Call the actual delete function
    return mvn_hmap_delete(hmap, &temp_key);
}

// --- Value Implementation ---

/** @brief Creates a NULL value. */
mvn_val_t mvn_val_null(void)
{
    return (mvn_val_t){.type = MVN_VAL_NULL};
}
/** @brief Creates a boolean value. */
mvn_val_t mvn_val_bool(bool b)
{
    return (mvn_val_t){.type = MVN_VAL_BOOL, .b = b}; // No .as
}
/** @brief Creates a 32-bit integer value. */
mvn_val_t mvn_val_i32(int32_t i32)
{
    return (mvn_val_t){.type = MVN_VAL_I32, .i32 = i32}; // No .as
}
/** @brief Creates a 64-bit integer value. */
mvn_val_t mvn_val_i64(int64_t i64)
{
    return (mvn_val_t){.type = MVN_VAL_I64, .i64 = i64}; // No .as
}
/** @brief Creates a 32-bit float value. */
mvn_val_t mvn_val_f32(float f32)
{
    return (mvn_val_t){.type = MVN_VAL_F32, .f32 = f32}; // No .as
}
/** @brief Creates a 64-bit double value. */
mvn_val_t mvn_val_f64(double f64)
{
    return (mvn_val_t){.type = MVN_VAL_F64, .f64 = f64}; // No .as
}

/** @brief Creates a value owning a new string copied from a C string. */
mvn_val_t mvn_val_string(const char *chars)
{
    mvn_string_t *s = mvn_string_new(chars);
    if (!s)
        return mvn_val_null();                            // Handle allocation failure
    return (mvn_val_t){.type = MVN_VAL_STRING, .str = s}; // No .as
}

/** @brief Creates a value taking ownership of an existing string. */
mvn_val_t mvn_val_string_take(mvn_string_t *str)
{
    if (!str)
        return mvn_val_null();
    return (mvn_val_t){.type = MVN_VAL_STRING, .str = str}; // No .as
}

/** @brief Creates a value owning a new empty array. */
mvn_val_t mvn_val_array(void)
{
    mvn_array_t *a = mvn_array_new();
    if (!a)
        return mvn_val_null();                           // Handle allocation failure
    return (mvn_val_t){.type = MVN_VAL_ARRAY, .arr = a}; // No .as
}

/** @brief Creates a value taking ownership of an existing array. */
mvn_val_t mvn_val_array_take(mvn_array_t *arr)
{
    if (!arr)
        return mvn_val_null();
    return (mvn_val_t){.type = MVN_VAL_ARRAY, .arr = arr}; // No .as
}

/** @brief Creates a value owning a new empty hash map. */
mvn_val_t mvn_val_hmap(void)
{
    mvn_hmap_t *m = mvn_hmap_new();
    if (!m)
        return mvn_val_null();                              // Handle allocation failure
    return (mvn_val_t){.type = MVN_VAL_HASHMAP, .hmap = m}; // No .as
}

/** @brief Creates a value taking ownership of an existing hash map. */
mvn_val_t mvn_val_hmap_take(mvn_hmap_t *hmap)
{
    if (!hmap)
        return mvn_val_null();
    return (mvn_val_t){.type = MVN_VAL_HASHMAP, .hmap = hmap}; // No .as
}

/**
 * @brief Frees the resources owned by a mvn_val_t.
 * If the value type is STRING, ARRAY, or HASHMAP, it frees the associated
 * dynamic structure recursively. For other types, it does nothing.
 * Resets the value to MVN_VAL_NULL after freeing.
 * @param value Pointer to the value to free. Does nothing if NULL.
 */
void mvn_val_free(mvn_val_t *value)
{
    if (!value)
        return;
    switch (value->type) {
        // Dynamic types that need freeing:
        case MVN_VAL_STRING:
            mvn_string_free(value->str); // No .as
            break;
        case MVN_VAL_ARRAY:
            mvn_array_free(value->arr); // No .as
            break;
        case MVN_VAL_HASHMAP:
            mvn_hmap_free(value->hmap); // No .as
            break;
        // Primitive types and NULL don't own heap resources:
        case MVN_VAL_NULL:
        case MVN_VAL_BOOL:
        case MVN_VAL_I32:
        case MVN_VAL_I64:
        case MVN_VAL_F32:
        case MVN_VAL_F64:
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
 * @brief Prints a representation of the value to stdout (for debugging).
 * @param value Pointer to the value to print. Handles NULL pointer.
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
            printf(value->b ? "true" : "false"); // No .as
            break;
        case MVN_VAL_I32:
            printf("%" PRId32, value->i32); // No .as
            break;                          // Use PRI macros for portability
        case MVN_VAL_I64:
            printf("%" PRId64, value->i64); // No .as
            break;
        case MVN_VAL_F32:
            printf("%g", value->f32); // No .as
            break;                    // %g is often suitable for floats/doubles
        case MVN_VAL_F64:
            printf("%g", value->f64); // No .as
            break;
        case MVN_VAL_STRING:
            printf("\"%s\"", value->str ? value->str->data : "NULL_STR_PTR"); // No .as
            break;
        case MVN_VAL_ARRAY:
            if (!value->arr) { // No .as
                printf("NULL_ARR_PTR");
                break;
            }
            printf("[");
            for (size_t i = 0; i < value->arr->count; i++) { // No .as
                mvn_val_print(&value->arr->data[i]);         // No .as
                if (i < value->arr->count - 1) {             // No .as
                    printf(", ");
                }
            }
            printf("]");
            break;
        case MVN_VAL_HASHMAP: { // Add opening brace
            if (!value->hmap) { // No .as
                printf("NULL_HMAP_PTR");
                break;
            }
            printf("{");
            bool first = true;
            for (size_t i = 0; i < value->hmap->capacity; i++) {   // No .as
                mvn_hmap_entry_t *entry = value->hmap->buckets[i]; // No .as
                while (entry) {
                    if (!first) {
                        printf(", ");
                    }
                    first = false;
                    // Assume key is always valid if entry exists
                    printf("\"%s\": ", entry->key->data);
                    mvn_val_print(&entry->value);
                    entry = entry->next;
                }
            }
            printf("}");
            break;
        } // Add closing brace
        default:
            printf("UNKNOWN_TYPE(%d)", value->type);
            break;
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // MVN_DS_C
