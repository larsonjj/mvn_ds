# Define the source directory for benchmark source files
set(MVN_BENCHMARK_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/source)

# Define the list of benchmark source files
set(MVN_BENCHMARK_SOURCES
    ${MVN_BENCHMARK_SOURCE_DIR}/mvn_ds_arr_benchmark.c
    ${MVN_BENCHMARK_SOURCE_DIR}/mvn_ds_hmap_benchmark.c
    ${MVN_BENCHMARK_SOURCE_DIR}/mvn_ds_string_benchmark.c
    ${MVN_BENCHMARK_SOURCE_DIR}/mvn_ds_primitives_benchmark.c # Add the new benchmark here
)

# List to store all benchmark targets
set(MVN_BENCHMARK_TARGETS)

foreach(BENCHMARK_SOURCE ${MVN_BENCHMARK_SOURCES})
    get_filename_component(BENCHMARK_NAME ${BENCHMARK_SOURCE} NAME_WE)
    add_executable(${BENCHMARK_NAME} ${BENCHMARK_SOURCE})
    target_include_directories(${BENCHMARK_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include # Include the main library headers
        ${CMAKE_CURRENT_SOURCE_DIR}/include    # Include benchmark-specific headers
    )
    target_link_libraries(${BENCHMARK_NAME} PRIVATE mvn_ds)
    list(APPEND MVN_BENCHMARK_TARGETS ${BENCHMARK_NAME})
endforeach()

# Create a meta-target for all benchmarks
add_custom_target(mvn_ds_benchmarks DEPENDS ${MVN_BENCHMARK_TARGETS})
set_target_properties(mvn_ds_benchmarks PROPERTIES EXCLUDE_FROM_ALL FALSE)
