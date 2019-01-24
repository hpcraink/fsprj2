if (WITH_UNIT_TESTS)
    find_library (CUNIT_LIB cunit)
    include_directories (${Cunit_INCLUDE_DIRS})

    # for linking against pthread in atomic_add_thread
    find_package(Threads REQUIRED)

    add_executable (atomic_add atomic_add.c) 
    target_include_directories (atomic_add PUBLIC ${PROJECT_SOURCE_DIR}/src)
    target_link_libraries (atomic_add PUBLIC cunit)
    # add_test ("Atomic Add single-threaded" atomic_add)

    add_executable (atomic_add_thread atomic_add_thread.c)
    target_include_directories (atomic_add_thread PUBLIC ${PROJECT_SOURCE_DIR}/src) 
    target_link_libraries (atomic_add_thread PUBLIC ${CMAKE_THREAD_LIBS_INIT} cunit)
    # add_test ("Atomic Add multi-threaded" atomic_add_thread)
endif ()
