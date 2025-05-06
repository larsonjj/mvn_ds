// filepath: tests/source/mvn_ds_cpp_test.cpp
/*
 * Copyright (c) 2024 Jake Larson
 */
#include <cstdlib> // For EXIT_SUCCESS, EXIT_FAILURE
#include <iostream>
#include <mvn_ds/mvn_ds_str.h> // Include one of the library headers

int main()
{
    mvn_string_t *test_string = mvn_string_new("Hello from C++"); // Call a C function

    if (test_string == NULL) {
        std::cerr << "Failed to create mvn_string from C++" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Successfully created mvn_string: " << test_string->data
              << std::endl; // Changed cstr to data

    mvn_string_free(test_string); // Call another C function

    return EXIT_SUCCESS;
}
