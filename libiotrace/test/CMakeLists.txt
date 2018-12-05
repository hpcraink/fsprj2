include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

enable_testing()

add_executable (read read.c) 
target_link_libraries(read libiotrace)

