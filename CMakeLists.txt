cmake_minimum_required(VERSION 3.22.1)

# Project information
project(mvn_ds
    VERSION 0.1.0
    DESCRIPTION "MVN Data Structures library"
    LANGUAGES C CXX)

# Options
option(MVN_DS_BUILD_TESTS "Build MVN_ds tests" ON)
option(MVN_DS_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)

# Suppress developer warnings
set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1 CACHE BOOL "Suppress developer warnings" FORCE)

# Disable installing to system directories.
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE INTERNAL "")

# Enable compilation database for tools like clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Define library sources
set(MVN_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/source/mvn_ds.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/mvn_ds_str.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/mvn_ds_arr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/mvn_ds_hmap.c
)

# Define library headers
set(MVN_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/mvn_ds/mvn_ds.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/mvn_ds/mvn_ds_str.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/mvn_ds/mvn_ds_arr.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/mvn_ds/mvn_ds_hmap.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/mvn_ds/mvn_ds_utils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/mvn_ds/mvn_ds_types.h
)

# Create library target
add_library(mvn_ds ${MVN_SOURCES})
add_library(mvn_ds::mvn_ds ALIAS mvn_ds)

# Set include directories
target_include_directories(mvn_ds
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source
)

# Set compiler warnings
if(MSVC)
    target_compile_options(mvn_ds PRIVATE
        /W4
        /wd4201  # Disable warning C4201 (nameless struct/union)
        /wd4312  # Disable warning C4312 (conversion to larger size)
        $<$<BOOL:${MVN_DS_WARNINGS_AS_ERRORS}>:/WX>)

    if(NOT CMAKE_GENERATOR STREQUAL "Ninja")
        target_compile_options(mvn_ds PRIVATE /MP)
    endif()
else()
    target_compile_options(mvn_ds PRIVATE
        -Wall -Wextra -Wpedantic
        $<$<BOOL:${MVN_DS_WARNINGS_AS_ERRORS}>:-Werror>)
endif()

# Link against SDL3 and SDL3_image for build-time usage
target_link_libraries(mvn_ds
  PUBLIC
    # For install-time usage - handled via find_dependency in config file
    $<INSTALL_INTERFACE:>
)

# Save the original link libraries before export
get_target_property(ORIGINAL_LINK_LIBS mvn_ds INTERFACE_LINK_LIBRARIES)

# When exporting the target, remove the SDL3 dependency
set_property(TARGET mvn_ds PROPERTY INTERFACE_LINK_LIBRARIES "")

# Installation rules
install(TARGETS mvn_ds
    EXPORT mvnDsTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Export targets
install(EXPORT mvnDsTargets
    FILE mvnDsTargets.cmake
    NAMESPACE mvn_ds::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mvn_ds
)

# Restore the SDL3 dependency after exporting
set_property(TARGET mvn_ds PROPERTY INTERFACE_LINK_LIBRARIES "${ORIGINAL_LINK_LIBS}")

# Create config file
include(CMakePackageConfigHelpers)
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/mvnDsConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/mvnDsConfig.cmake"
    @ONLY
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/mvnDsConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/mvnDsConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/mvnDsConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mvn_ds
)

# Process subdirectories if enabled
if(MVN_DS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
