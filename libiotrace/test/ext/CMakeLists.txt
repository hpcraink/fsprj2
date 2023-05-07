# The following workaround (to build cunit with Debug build type to ensure
# generation of assert functions) only works because cunit is added as a
# subdirectory. Each target uses the value a variable has at the end of the
# directory scope. A subdirectory has it's own scope, so at it's end the
# build type is the type defined before the add_subdirectory call (if no
# other definition inside the subdirectory is given).
set(CMAKE_BUILD_TYPE Debug)

# Warnings from compiling CUnit should be ignored (CUnit is not part of our production code).
add_compile_options(-Wno-pedantic -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-parameter -Wno-missing-field-initializers -Wno-format -Wno-strict-prototypes -Wno-unused-macros -Wno-switch-enum)

add_subdirectory(cunit)