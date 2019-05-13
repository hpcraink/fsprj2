enable_testing()

add_subdirectory(internal)

# ToDo: generate file with set LINK_OPTIONS and include it here

set(LINK_OPTIONS
    "-Wl,-wrap=open"
    "-Wl,-wrap=close"
    "-Wl,-wrap=fopen"
    "-Wl,-wrap=fdopen"
    "-Wl,-wrap=freopen"
    "-Wl,-wrap=fclose"
)

# test with: LD_PRELOAD=../src/libiotrace_shared.so ./read_byte
add_executable(read_byte_shared read_byte.c)
#target_link_libraries(read_byte iotrace)
#add_test ("Read" read_byte)

add_library(read_byte_object OBJECT read_byte.c)

# test with: ./read_byte_static
add_executable(read_byte_static
    $<TARGET_OBJECTS:read_byte_object>
    $<TARGET_OBJECTS:iotrace_object>
)
target_link_libraries(read_byte_static ${LINK_OPTIONS})

# test with: LD_PRELOAD=../src/libiotrace_shared.so ./read_stream
add_executable(read_stream_shared read_stream.c)

add_library(read_stream_object OBJECT read_stream.c)

# test with: ./read_stream_static
add_executable(read_stream_static
    $<TARGET_OBJECTS:read_stream_object>
    $<TARGET_OBJECTS:iotrace_object>
)
target_link_libraries(read_stream_static ${LINK_OPTIONS})

#test with: LD_PRELOAD=../src/libiotrace_shared.so OMP_NUM_THREADS=2 ./read_stream_openmp_shared
add_executable(read_stream_openmp_shared read_stream_openmp.c)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(read_stream_openmp_shared PUBLIC OpenMP::OpenMP_CXX)
endif()