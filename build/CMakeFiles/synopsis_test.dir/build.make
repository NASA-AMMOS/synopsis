# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.21

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.21.3_1/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.21.3_1/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/jackal/Documents/SYNOPSIS/synopsis

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/jackal/Documents/SYNOPSIS/synopsis/build

# Include any dependencies generated for this target.
include CMakeFiles/synopsis_test.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/synopsis_test.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/synopsis_test.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/synopsis_test.dir/flags.make

CMakeFiles/synopsis_test.dir/test/synopsis_test.cpp.o: CMakeFiles/synopsis_test.dir/flags.make
CMakeFiles/synopsis_test.dir/test/synopsis_test.cpp.o: ../test/synopsis_test.cpp
CMakeFiles/synopsis_test.dir/test/synopsis_test.cpp.o: CMakeFiles/synopsis_test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/jackal/Documents/SYNOPSIS/synopsis/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/synopsis_test.dir/test/synopsis_test.cpp.o"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/synopsis_test.dir/test/synopsis_test.cpp.o -MF CMakeFiles/synopsis_test.dir/test/synopsis_test.cpp.o.d -o CMakeFiles/synopsis_test.dir/test/synopsis_test.cpp.o -c /Users/jackal/Documents/SYNOPSIS/synopsis/test/synopsis_test.cpp

CMakeFiles/synopsis_test.dir/test/synopsis_test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/synopsis_test.dir/test/synopsis_test.cpp.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/jackal/Documents/SYNOPSIS/synopsis/test/synopsis_test.cpp > CMakeFiles/synopsis_test.dir/test/synopsis_test.cpp.i

CMakeFiles/synopsis_test.dir/test/synopsis_test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/synopsis_test.dir/test/synopsis_test.cpp.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/jackal/Documents/SYNOPSIS/synopsis/test/synopsis_test.cpp -o CMakeFiles/synopsis_test.dir/test/synopsis_test.cpp.s

# Object files for target synopsis_test
synopsis_test_OBJECTS = \
"CMakeFiles/synopsis_test.dir/test/synopsis_test.cpp.o"

# External object files for target synopsis_test
synopsis_test_EXTERNAL_OBJECTS =

synopsis_test: CMakeFiles/synopsis_test.dir/test/synopsis_test.cpp.o
synopsis_test: CMakeFiles/synopsis_test.dir/build.make
synopsis_test: lib/libgtest_main.a
synopsis_test: libsynopsis.0.8.0.dylib
synopsis_test: lib/libgtest.a
synopsis_test: CMakeFiles/synopsis_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/jackal/Documents/SYNOPSIS/synopsis/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable synopsis_test"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/synopsis_test.dir/link.txt --verbose=$(VERBOSE)
	/usr/local/Cellar/cmake/3.21.3_1/bin/cmake -D TEST_TARGET=synopsis_test -D TEST_EXECUTABLE=/Users/jackal/Documents/SYNOPSIS/synopsis/build/synopsis_test -D TEST_EXECUTOR= -D TEST_WORKING_DIR=/Users/jackal/Documents/SYNOPSIS/synopsis/build -D TEST_EXTRA_ARGS= -D "TEST_PROPERTIES=ENVIRONMENT;SYNOPSIS_TEST_DATA=/Users/jackal/Documents/SYNOPSIS/synopsis/test/data" -D TEST_PREFIX= -D TEST_SUFFIX= -D NO_PRETTY_TYPES=FALSE -D NO_PRETTY_VALUES=FALSE -D TEST_LIST=synopsis_test_TESTS -D CTEST_FILE=/Users/jackal/Documents/SYNOPSIS/synopsis/build/synopsis_test[1]_tests.cmake -D TEST_DISCOVERY_TIMEOUT=5 -D TEST_XML_OUTPUT_DIR= -P /usr/local/Cellar/cmake/3.21.3_1/share/cmake/Modules/GoogleTestAddTests.cmake

# Rule to build all files generated by this target.
CMakeFiles/synopsis_test.dir/build: synopsis_test
.PHONY : CMakeFiles/synopsis_test.dir/build

CMakeFiles/synopsis_test.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/synopsis_test.dir/cmake_clean.cmake
.PHONY : CMakeFiles/synopsis_test.dir/clean

CMakeFiles/synopsis_test.dir/depend:
	cd /Users/jackal/Documents/SYNOPSIS/synopsis/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/jackal/Documents/SYNOPSIS/synopsis /Users/jackal/Documents/SYNOPSIS/synopsis /Users/jackal/Documents/SYNOPSIS/synopsis/build /Users/jackal/Documents/SYNOPSIS/synopsis/build /Users/jackal/Documents/SYNOPSIS/synopsis/build/CMakeFiles/synopsis_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/synopsis_test.dir/depend

