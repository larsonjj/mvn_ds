Please help me generate an implementation a generic set of data structures that:

- Is compatible with C99 programming language
- Can handle all primitive types (int, float, double, etc), dynamic arrays, dynamic string, string key hashmaps
- Ensures the dynamic arrays and string key hashmaps allow for infinite nesting and should clean up their memory easily
- Uses ownership semantics for memory management, meaning that the data structures should take ownership of the data they contain and be responsible for freeing it when no longer needed
- Is easy to use and understand, with clear and concise APIs for creating, manipulating, and destroying the data structures
- Is well-documented with comments explaining the purpose and usage of each function and data structure. Use doxygen comments for implementation only
- Is designed to be extensible, allowing for future enhancements and additional features without breaking existing functionality

Use these data structures as inspiration:

```c
// --- Type Enum ---
typedef enum {
    MVN_VAL_NULL,
    MVN_VAL_BOOL,
    MVN_VAL_I32,
    MVN_VAL_I64,
    MVN_VAL_UI32,
    MVN_VAL_UI64,
    MVN_VAL_F32,
    MVN_VAL_F64,
    MVN_VAL_STRING, // Owned mvn_string_t*
    MVN_VAL_ARRAY,  // Owned mvn_array_t*
    MVN_VAL_HASHMAP // Owned Hashmap*
} mvn_val_type_t;

// --- Generic Value ---
typedef struct mvn_val_t {
    mvn_val_type_t type;
    union {
        bool bool;
        int32_t i32;
        int64_t i64;
        float f32;
        double f64;
        String* str;
        Array* arr;
        Hashmap* hmap;
    };
} mvn_val_t;

// --- Dynamic String ---
typedef struct mvn_string_t {
    size_t length;
    size_t capacity;
    char* data; // Null-terminated
} mvn_string_t;

// --- Dynamic Array ---
typedef struct mvn_array_t {
    size_t count;
    size_t capacity;
    mvn_val_t* data; // mvn_array_t of mvn_val_t structs
} mvn_array_t;

// --- Hash Map ---
typedef struct mvn_hmap_entry_t {
    mvn_string_t* key; // Owned mvn_string_t*
    mvn_val_t value; // Owned Value
    mvn_hmap_entry_t* next; // For collision chaining
} mvn_hmap_entry_t;

typedef struct mvn_hmap_t {
    size_t count;
    size_t capacity;
    mvn_hmap_entry_t** buckets;
} mvn_hmap_t;
```

Please provide implementation code and example usage on how to create a complex structure similar to the JSON object like the following:

```json
{
  "name": "Example Project",
  "version": 1.2,
  "enabled": true,
  "components": [
    {"id": 101, "type": "sensor"},
    {"id": 102, "type": "actuator"}
  ],
  "metadata": null
}
```
