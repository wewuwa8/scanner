add_executable(scnr_lib_tests all_tests.cpp)
target_link_libraries(scnr_lib_tests scnr gtest_main)
add_test(NAME scnr_lib_tests COMMAND scnr_lib_tests WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/data")