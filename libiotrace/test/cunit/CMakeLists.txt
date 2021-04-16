enable_testing()

add_executable(test_utils test_utils.c ../../src/utils.c)
set_target_properties(test_utils PROPERTIES LINK_FLAGS -W)
target_link_libraries(test_utils PRIVATE cunit)

add_test(NAME test_utils COMMAND test_utils)

add_executable(test_libiotrace_structs test_libiotrace_structs.c)
set_target_properties(test_libiotrace_structs PROPERTIES LINK_FLAGS -W)
target_compile_options(test_libiotrace_structs PRIVATE ${OPTIONS})
target_link_libraries(test_libiotrace_structs PRIVATE cunit)

add_test(NAME test_libiotrace_structs COMMAND test_libiotrace_structs)

if(WITH_ALLOC)
	add_executable(test_alloc test_alloc.c ../../src/alloc.c ../../src/utils.c)
	set_target_properties(test_alloc PROPERTIES LINK_FLAGS -W)
	target_compile_options(test_alloc PRIVATE ${OPTIONS})
	target_link_libraries(test_alloc PRIVATE cunit -ldl)

	add_test(NAME test_alloc COMMAND test_alloc)
endif()
