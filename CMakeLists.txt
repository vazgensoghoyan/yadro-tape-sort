cmake_minimum_required(VERSION 3.20)

project(tape_sort LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_WARNINGS "Enable compiler warnings" ON)
option(BUILD_TESTS "Build tests" ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

function(enable_project_warnings target)
    if (ENABLE_WARNINGS)
        if (MSVC)
            target_compile_options(${target} PRIVATE
                /W4
                /permissive-
            )
        else()
            target_compile_options(${target} PRIVATE
                -Wall
                -Wextra
                -Wpedantic
                -Wconversion
                -Wsign-conversion
            )
        endif()
    endif()
endfunction()

file(GLOB_RECURSE TAPE_SORT_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

list(FILTER TAPE_SORT_SOURCES EXCLUDE REGEX ".*main\\.cpp$")

add_library(tape_sort_lib ${TAPE_SORT_SOURCES})

target_include_directories(tape_sort_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(tape_sort_lib PUBLIC cxx_std_23)

enable_project_warnings(tape_sort_lib)

add_executable(tape_sort
    src/main.cpp
)

target_link_libraries(tape_sort
    PRIVATE
        tape_sort_lib
)

target_compile_features(tape_sort PUBLIC cxx_std_23)

enable_project_warnings(tape_sort)

if (BUILD_TESTS)
    enable_testing()

    include(FetchContent)

    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(googletest)

    file(GLOB_RECURSE TEST_SOURCES
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
    )

    add_executable(tape_sort_tests
        ${TEST_SOURCES}
    )

    target_link_libraries(tape_sort_tests
        PRIVATE
            tape_sort_lib
            GTest::gtest_main
    )

    target_compile_features(tape_sort_tests PUBLIC cxx_std_23)

    enable_project_warnings(tape_sort_tests)

    include(GoogleTest)

    gtest_discover_tests(tape_sort_tests)
endif()
