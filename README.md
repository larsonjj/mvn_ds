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
               mvn_str_get_data(my_string),
               mvn_str_get_length(my_string),
               mvn_str_get_capacity(my_string));

        mvn_str_append_cstr(my_string, " Appended text.");
        printf("Updated String: %s\n", mvn_str_get_data(my_string));

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
        printf("String value: %s\n", mvn_str_get_data(val_message.str));
    }

    // mvn_val_free will handle freeing the string data for val_message
    mvn_val_free(&val_int);     // Resets type to MVN_VAL_NULL
    mvn_val_free(&val_message); // Frees internal string and resets type

    return 0;
}
```

> NOTE: Refer to the header files in `include/mvn_ds` for detailed API documentation of each data structure and function. Doxygen-generated documentation can be built by running `doxygen Doxyfile` from the project root (if Doxygen is installed).

## Contributing

Contributions are welcome! Please refer to the issue tracker for open issues or to submit new ones. Pull requests should follow the existing code style (see [docs/code_style.md](/Users/validmac/Projects/c_cpp/mvn_ds/docs/code_style.md)) and ensure all tests pass.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
