# Fuzzy test project
# @author Maxim Menshikov (maxim@menshikov.org)
cmake_minimum_required(VERSION 3.0)

project(fuzzytest)

set(SRC src/main.cpp
        src/Generator.cpp)

option(FUZZYTEST_ENABLE_CLANG_TIDY "Enable codegen clang-tidy"  OFF)

add_executable(${PROJECT_NAME} ${SRC})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} stdc++fs)

if (FUZZYTEST_ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_BINARY NAMES "clang-tidy")
    if (CLANG_TIDY_BINARY)
        set_target_properties(${PROJECT_NAME} PROPERTIES
                              CXX_CLANG_TIDY "${CLANG_TIDY_BINARY}")
    else ()
        message(STATUS "clang-tidy not found")
    endif()
endif()
