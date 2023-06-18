cmake_minimum_required(VERSION 3.14)

project(scanner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message("CMAKE_MODULE_PATH = ${CMAKE_MODULE_PATH}")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# if (tests)
enable_testing()

# endif()
add_subdirectory(lib)
add_subdirectory(src)

message("TopLevel build done!")