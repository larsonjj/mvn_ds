##### TEST SUITE MACRO #####
function(mvn_ds_add_test_executable target source)
    add_executable(${target} ${source})
    add_dependencies(${target} copy_test_assets)
    target_compile_definitions(${target} PRIVATE
        ASSET_DIR="${CMAKE_BINARY_DIR}/tests/assets"
    )
    target_include_directories(${target} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(${target} PRIVATE mvn)
    set_target_properties(${target} PROPERTIES
        C_STANDARD 99
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF
    )
endfunction()

##### TEST CONFIGURATION #####
# List of all test modules
set(MVN_DS_TEST_MODULES
    array
    hmap
    primitives
    string
)

# Build all test executables
set(MVN_DS_TEST_TARGETS)
foreach(MODULE ${MVN_DS_TEST_MODULES})
    set(TARGET_NAME mvn_ds_test_${MODULE})
    list(APPEND MVN_DS_TEST_TARGETS ${TARGET_NAME})
    mvn_ds_add_test_executable(${TARGET_NAME} source/mvn_ds_${MODULE}_test.c)
endforeach()

# Create a meta-target that depends on all test executables
add_custom_target(mvn_ds_test_build_all DEPENDS ${MVN_DS_TEST_TARGETS})

# Make this meta-target part of the default build
set_target_properties(mvn_ds_test_build_all PROPERTIES EXCLUDE_FROM_ALL FALSE)

# Make sure the meta-target depends on the main mvn target
add_dependencies(mvn_ds_test_build_all mvn)

##### TESTS #####
# Register all tests with CTest
foreach(TARGET ${MVN_DS_TEST_TARGETS})
    add_test(NAME ${TARGET} COMMAND ${TARGET})
endforeach()
