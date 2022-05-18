enable_testing()

#
# IMPORTANT: Tests which shall be run by Github's CI Pipeline MUST use `test_` as prefix in the filename
#

# - CTest config
# if(${CMAKE_VERSION} VERSION_LESS "3.12.0") 		# Doesn't work (https://stackoverflow.com/a/5757256) ?
#   list(APPEND CMAKE_CTEST_ARGUMENTS "--output-on-failure")
# else()
#   set(CTEST_OUTPUT_ON_FAILURE=1) 	# this is for new versions of cmake
# endif()
# - CTest config


add_executable(test_utils test_utils.c ../../src/common/utils.c)
set_target_properties(test_utils PROPERTIES LINK_FLAGS -W)
target_link_libraries(test_utils PRIVATE cunit)

add_executable(test_gettime test_gettime.c ../../src/common/gettime.c)
set_target_properties(test_gettime PROPERTIES LINK_FLAGS -W)
target_link_libraries(test_gettime PRIVATE cunit)

add_test(NAME test_utils COMMAND test_utils)
add_test(NAME test_gettime COMMAND test_gettime)

add_executable(
		test_libiotrace_structs
		test_libiotrace_structs.c
		../../src/libiotrace_defines_utils.c
		../../src/libs/line_protocol_parser.c
)
set_target_properties(test_libiotrace_structs PROPERTIES LINK_FLAGS -W)
#list(APPEND LIBIOTRACE_COMPILE_OPTIONS -fsanitize=undefined)
target_compile_options(test_libiotrace_structs PRIVATE ${LIBIOTRACE_COMPILE_OPTIONS})
#list(REMOVE_ITEM LIBIOTRACE_COMPILE_OPTIONS -fsanitize=undefined)
#target_link_libraries(test_libiotrace_structs PRIVATE cunit ubsan)
target_link_libraries(test_libiotrace_structs PRIVATE cunit)

add_test(NAME test_libiotrace_structs COMMAND test_libiotrace_structs)

if(WITH_ALLOC)
	# don't include POSIX wrapper during compile of ../../src/alloc.c
	list(REMOVE_ITEM LIBIOTRACE_COMPILE_OPTIONS -DWITH_POSIX_IO)

	# we need exact one output function
	list(REMOVE_ITEM LIBIOTRACE_COMPILE_OPTIONS -DENABLE_OUTPUT=${ENABLE_OUTPUT})
	list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DENABLE_OUTPUT=LOGFILE)

	# generate functions with prefix "__test_" instead of alloc wrappers
	# (to prevent wrapping of alloc functions in cunit framework and
	# enable testing of wrappers with dedicated function calls)
	list(APPEND LIBIOTRACE_COMPILE_OPTIONS -DIO_LIB_TEST)

	add_executable(test_alloc test_alloc.c ../../src/alloc.c ../../src/common/utils.c ../../src/libiotrace_defines_utils.c ../../src/common/gettime.c)
	set_target_properties(test_alloc PROPERTIES LINK_FLAGS -W)
	target_compile_options(test_alloc PRIVATE ${LIBIOTRACE_COMPILE_OPTIONS})
	target_link_libraries(test_alloc PRIVATE cunit -ldl)

	add_test(NAME test_alloc COMMAND test_alloc)
endif()



if (FILENAME_RESOLUTION_ENABLED)
	set(CMAKE_C_FLAGS "-D_GNU_SOURCE -DFILENAME_RESOLUTION_ENABLED")

# - fctevent -
	add_executable(fnres_test_fctevent fnres/test_fnres.c
			../../src/fnres/fnmap/fnmap_atomic_hash.c
            ../../src/fnres/fnres.c)
	set_target_properties(fnres_test_fctevent PROPERTIES LINK_FLAGS -W)
	target_link_libraries(fnres_test_fctevent PRIVATE cunit atomic_hash_object)
	add_test(NAME fnres_test_fctevent COMMAND fnres_test_fctevent)


# - fnmap -
	add_executable(fnres_validate_fnmap fnres/validate_fnmap.c
			../../src/fnres/fnmap/fnmap_atomic_hash.c)
	set_target_properties(fnres_validate_fnmap PROPERTIES LINK_FLAGS -W)
	target_link_libraries(fnres_validate_fnmap PRIVATE cunit atomic_hash_object)
	add_test(NAME fnres_validate_fnmap COMMAND fnres_validate_fnmap)
endif()
