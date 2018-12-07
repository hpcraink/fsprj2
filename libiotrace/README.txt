Library for IO tracing 
----------------------

This library linked against or LD_PRELOADed to executables
catches IO calls of all sub-threads and writes those to files.


BUILDING
--------
As usual with CMake, out-of-source builds are recommended:
this cleanly separates the sources from any artefacts such as
Makefiles, object files and executables.

NetBeans:
   In "File->New Project" select "C/C++ Project with Existing Source".
   and "Browse" to the source location.
   (CMake needs to be configured in Your NetBeans Preferences under C/C++)
   In Automatic mode, Netbeans will detect the CMakeLists.txt file
   and build and install the library.

Unix-Makefiles:
   mkdir BUILD && cd BUILD
   cmake ..
   make

Eclipse:
   mkdir BUILD && cd BUILD
   cmake -G "Eclipse CDT4 - Unix Makefiles" ..
   Then import in Eclipse.

OSX XCode:
   mkdir BUILD && cd BUILD
   cmake -G Xcode ..
   open HFTOpenCL.xcodeproj

Netbeans:
Follow the instructions for standard Unix-Makefiles, then
in Netbeans create a new C/C++ Project based on existing sources.


CLEANUP:
--------
If You have run cmake and build IN the main project directory,
then delete all generated files manually using:

# Equivalent to a standard make distclean
rm -fr CMakeCache.txt CMakeFiles/ Makefile cmake_install.cmake install_manifest.txt \
       test/CMakeFiles/ test/CTestTestfile.cmake test/cmake_install.cmake test/Makefile \
       src/CMakeFiles/ src/cmake_install.cmake src/Makefile
rm -f include/libiotrace_config.h src/libiotrace.a
# In case of any BUILD-directories:
rm -fr BUILD*/
# In case of Netbeans:
rm -fr nbproject/
rm -f compile_commands.json
# For Mac OSX file:
find . -name .DS_Store | xargs rm


DIRECTORIES:
------------

config/   all additional CMake-files required to build, test or install
include/  header files with external visibility
src/      contains all the source
test/     internal tests

