# MVN Data Structures (mvn_ds)

A C99 compatible library providing a generic set of data structures, including dynamic arrays, dynamic strings, and string-key hash maps, with support for primitive types and nested structures.

## Description

`mvn_ds` is designed to be an easy-to-use and extensible data structures library for C. It focuses on clear APIs, ownership semantics for memory management, and compatibility across common C compilers (MSVC, GCC, Clang). The library aims to simplify common data manipulation tasks in C projects.

Key data types include:

- `mvn_val_t`: A tagged union type that can hold various primitive types (`NULL`, `bool`, `int8_t`, `int16_t`, `int32_t`, `int64_t`, `uint8_t`, `uint16_t`, `uint32_t`, `uint64_t`, `float`, `double`, `char`, `void*`), as well as dynamic `mvn_str_t`, `mvn_arr_t`, and `mvn_hmap_t`.
- `mvn_str_t`: A dynamic string implementation.
- `mvn_arr_t`: A dynamic array (vector) implementation capable of storing `mvn_val_t` values, allowing for heterogeneous collections and nesting.
- `mvn_hmap_t`: A hash map implementation using `mvn_str_t` keys and storing `mvn_val_t` values, also supporting nesting.

## Features

- **C99 Compatible**: Designed to work with standard C99.
- **Generic Value Type**: `mvn_val_t` can hold primitives, strings, arrays, and hash maps.
- **Dynamic Collections**:
  - Dynamic strings (`mvn_str_t`)
  - Dynamic arrays (`mvn_arr_t`)
  - String-key hash maps (`mvn_hmap_t`)
- **Nesting**: Arrays and hash maps can contain other arrays and hash maps, allowing for complex, nested data structures.
- **Ownership Semantics**: Data structures take ownership of the dynamic data they contain and are responsible for freeing it.
- **Memory Management**: Uses configurable memory management functions (defaults to standard `malloc`, `calloc`, `realloc`, `free`, but can be aliased, e.g., via `MVN_DS_MALLOC`).
- **Cross-Compiler Compatibility**: Aims for compatibility with MSVC, GCC, and Clang.
- **Well-Documented**: Doxygen comments in implementation files.
- **Extensible Design**: Built with future type enhancements in mind.

## Building the Project

This project uses CMake for building.

1. **Prerequisites**:
    - CMake (version 3.22.1 or higher recommended)
    - A C99 compatible C compiler (e.g., GCC, Clang, MSVC)
    - (Optional for C++ tests) A C++ compiler

2. **Configure**:
    Create a build directory and run CMake from there:

    ```bash
    mkdir build
    cd build
    cmake ..
    ```

    You can specify a generator if needed (e.g., `-G "Ninja"` or `-G "Visual Studio 17 2022"`).

3. **Build**:

    ```bash
    cmake --build .
    ```

    Or use the native build system command (e.g., `ninja` or `make`).

    This will build the `mvn_ds` static library.

## Running Tests

Tests are built if `MVN_DS_BUILD_TESTS` option is `ON` (default).

1. Build the project (including tests) as described above.
2. From the build directory, run CTest:

    ```bash
    ctest --output-on-failure
    ```

    To run a specific test (e.g., `mvn_ds_str_test`):

    ```bash
    ctest --output-on-failure -R ^mvn_ds_str_test$
    ```

    Memory checking can be performed using tools like Valgrind, as configured in the GitHub Actions workflow (see [`.github/workflows/memory-check.yml`](.github/workflows/memory-check.yml)).

## Usage

Include the main library header:

```c
#include <mvn_ds/mvn_ds.h>
```

Example (basic string usage):

```c
#include <mvn_ds/mvn_ds.h>
#include <stdio.h>

int main() {
    mvn_str_t* my_string = mvn_str_new("Hello, mvn_ds!");
    if (my_string) {
        printf("String: %s (Length: %zu, Capacity: %zu)\n",
               mvn_str_data(my_string),
               mvn_str_length(my_string),
               mvn_str_capacity(my_string));

        mvn_str_append_cstr(my_string, " Appended text.");
        printf("Updated String: %s\n", mvn_str_data(my_string));

        mvn_str_free(my_string);
    }
    return 0;
}
```

Example (basic mvn_val_t usage):

```c
#include <mvn_ds/mvn_ds.h>
#include <stdio.h>

int main() {
    mvn_val_t val_int = mvn_val_i32(123);
    mvn_val_t val_message = mvn_val_str("This is a test message.");

    if (val_int.type == MVN_VAL_I32) {
        printf("Integer value: %d\n", val_int.i32);
    }

    if (val_message.type == MVN_VAL_STRING && val_message.str != NULL) {
        printf("String value: %s\n", mvn_str_data(val_message.str));
    }

    // mvn_val_free will handle freeing the string data for val_message
    mvn_val_free(&val_int);     // Resets type to MVN_VAL_NULL
    mvn_val_free(&val_message); // Frees internal string and resets type

    return 0;
}
```

Example (Extended `mvn_val_t` Types Usage):

```c
#include <mvn_ds/mvn_ds.h>
#include <stdio.h>

int main() {
    mvn_val_t val_bool_true = mvn_val_bool(true);
    mvn_val_t val_double_pi = mvn_val_f64(3.1415926535);
    mvn_val_t val_char_z    = mvn_val_char('Z');
    int       dummy         = 42;
    mvn_val_t val_pointer   = mvn_val_ptr(&dummy);

    if (val_bool_true.type == MVN_VAL_BOOL) {
        printf("Boolean value: %s\n", val_bool_true.b ? "true" : "false");
    }
    if (val_double_pi.type == MVN_VAL_F64) {
        printf("Double value: %f\n", val_double_pi.f64);
    }
    if (val_char_z.type == MVN_VAL_CHAR) {
        printf("Char value: %c\n", val_char_z.c);
    }
    if (val_pointer.type == MVN_VAL_PTR) {
        printf("Pointer value: %p (points to %d)\n", val_pointer.ptr, *(int*)val_pointer.ptr);
    }

    mvn_val_free(&val_bool_true);
    mvn_val_free(&val_double_pi);
    mvn_val_free(&val_char_z);
    mvn_val_free(&val_pointer); // Does not free the memory pointed to by &dummy

    return 0;
}
```

Example (Basic Array `mvn_arr_t` Usage):

```c
#include <mvn_ds/mvn_ds.h>
#include <stdio.h>

int main() {
    mvn_arr_t* my_array = mvn_arr_new();
    if (!my_array) return 1;

    mvn_arr_push(my_array, mvn_val_i32(10));
    mvn_arr_push(my_array, mvn_val_str("An array element"));
    mvn_arr_push(my_array, mvn_val_bool(false));

    printf("Array count: %zu\n", mvn_arr_count(my_array));

    for (size_t i = 0; i < mvn_arr_count(my_array); ++i) {
        mvn_val_t* val_item = mvn_arr_get(my_array, i);
        printf("Array[%zu]: ", i);
        mvn_val_print(val_item); // Simple print for demonstration
        printf("\n");
    }

    mvn_arr_free(my_array); // Frees the array and all its owned elements
    return 0;
}
```

Example (Basic Hash Map `mvn_hmap_t` Usage):

```c
#include <mvn_ds/mvn_ds.h>
#include <stdio.h>

int main() {
    mvn_hmap_t* my_map = mvn_hmap_new();
    if (!my_map) return 1;

    // Using mvn_hmap_set_cstr for convenience with C string keys
    mvn_hmap_set_cstr(my_map, "name", mvn_val_str("mvn_ds_user"));
    mvn_hmap_set_cstr(my_map, "version", mvn_val_f32(0.1f));
    mvn_hmap_set_cstr(my_map, "active", mvn_val_bool(true));

    printf("Map count: %zu\n", mvn_hmap_count(my_map));

    mvn_val_t* name_val = mvn_hmap_cstr(my_map, "name");
    if (name_val && name_val->type == MVN_VAL_STRING) {
        printf("Name: %s\n", mvn_str_data(name_val->str));
    }

    mvn_val_t* version_val = mvn_hmap_cstr(my_map, "version");
    if (version_val && version_val->type == MVN_VAL_F32) {
        printf("Version: %.1f\n", version_val->f32);
    }

    mvn_hmap_free(my_map); // Frees the map, its keys, and all its owned values
    return 0;
}
```

Example (Complex Nested Data Structure - JSON-like):

```c
#include <mvn_ds/mvn_ds.h>
#include <stdio.h>

int main() {
    // Create the top-level structure (a hash map)
    mvn_val_t root_val = mvn_val_hmap(); // Creates an owned hmap within mvn_val_t
    if (root_val.type != MVN_VAL_HASHMAP || !root_val.hmap) {
        fprintf(stderr, "Failed to create root hash map value.\n");
        return 1;
    }
    mvn_hmap_t* root_map = root_val.hmap;

    // Add some simple key-value pairs
    mvn_hmap_set_cstr(root_map, "projectName", mvn_val_str("mvn_ds Project"));
    mvn_hmap_set_cstr(root_map, "version", mvn_val_i32(1));
    mvn_hmap_set_cstr(root_map, "isActive", mvn_val_bool(true));
    mvn_hmap_set_cstr(root_map, "rating", mvn_val_f64(4.5));

    // Create a nested array
    mvn_val_t items_array_val = mvn_val_arr(); // Creates an owned array
    if (items_array_val.type != MVN_VAL_ARRAY || !items_array_val.arr) {
        fprintf(stderr, "Failed to create items array value.\n");
        mvn_val_free(&root_val);
        return 1;
    }
    mvn_arr_t* items_array = items_array_val.arr;

    mvn_arr_push(items_array, mvn_val_str("First Item"));
    mvn_arr_push(items_array, mvn_val_i32(101));
    mvn_arr_push(items_array, mvn_val_bool(false));

    // Create a small map to nest inside the array
    mvn_val_t nested_map_in_array_val = mvn_val_hmap();
    mvn_hmap_set_cstr(nested_map_in_array_val.hmap, "status", mvn_val_str("pending"));
    mvn_arr_push(items_array, nested_map_in_array_val); // Add map to array

    // Add the items array to the root map
    mvn_hmap_set_cstr(root_map, "featureList", items_array_val);

    // Create another nested hash map for 'details'
    mvn_val_t details_map_val = mvn_val_hmap(); // Creates an owned map
    if (details_map_val.type != MVN_VAL_HASHMAP || !details_map_val.hmap) {
        fprintf(stderr, "Failed to create details map value.\n");
        mvn_val_free(&root_val); // root_val owns items_array_val at this point
        return 1;
    }
    mvn_hmap_t* details_map = details_map_val.hmap;

    mvn_hmap_set_cstr(details_map, "author", mvn_val_str("Jake"));
    mvn_hmap_set_cstr(details_map, "description", mvn_val_str("A complex nested data example."));
    
    // Create an array within the details map
    mvn_val_t contributors_val = mvn_val_arr();
    mvn_arr_push(contributors_val.arr, mvn_val_str("ContributorA"));
    mvn_arr_push(contributors_val.arr, mvn_val_str("ContributorB"));
    mvn_hmap_set_cstr(details_map, "contributors", contributors_val);


    // Add the details map to the root map
    mvn_hmap_set_cstr(root_map, "projectDetails", details_map_val);

    // Print the entire structure
    printf("Complex Data Structure:\n");
    mvn_val_print(&root_val);
    printf("\n");

    // Free the root value; all nested owned structures will be freed recursively
    mvn_val_free(&root_val);

    return 0;

// This example builds a data structure in C that would be analogous to
// the following JSON object:
//
// {
//   "projectName": "mvn_ds Project",
//   "version": 1,
//   "isActive": true,
//   "rating": 4.5,
//   "featureList": [
//     "First Item",
//     101,
//     false,
//     {
//       "status": "pending"
//     }
//   ],
//   "projectDetails": {
//     "author": "Jake",
//     "description": "A complex nested data example.",
//     "contributors": [
//       "ContributorA",
//       "ContributorB"
//     ]
//   }
// }
}
```

> NOTE: Refer to the header files in `include/mvn_ds` for detailed API documentation of each data structure and function. Doxygen-generated documentation can be built by running `doxygen Doxyfile` from the project root (if Doxygen is installed).

## Contributing

Contributions are welcome! Please refer to the issue tracker for open issues or to submit new ones. Pull requests should follow the existing code style (see [docs/code_style.md](/Users/validmac/Projects/c_cpp/mvn_ds/docs/code_style.md)) and ensure all tests pass.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
