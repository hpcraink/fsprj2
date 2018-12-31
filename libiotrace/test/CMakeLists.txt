enable_testing()

add_subdirectory (internal)

add_executable (read read.c) 
target_link_libraries(read libiotrace)
add_test ("Read" read)

