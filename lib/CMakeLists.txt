find_package(Threads REQUIRED)
message("CMAKE_MODULE_PATH = ${CMAKE_MODULE_PATH}")
add_library(scnr STATIC
    scnr.cpp
    thread_pool.cpp
    parse_elf.cpp
    parse_mach-o.cpp
    parse_pe.cpp
    parse_encoding.cpp
    parse_xml.cpp
    context.cpp
)
target_link_libraries(scnr PUBLIC Threads::Threads)
target_include_directories(scnr PUBLIC include)

# if(tests)
include(googletest)
enable_testing()
add_subdirectory(tests)

# endif()