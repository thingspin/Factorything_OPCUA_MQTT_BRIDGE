# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.7

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/pi/paho.mqtt.embedded-c

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/pi/paho.mqtt.embedded-c/build

# Include any dependencies generated for this target.
include CMakeFiles/pub0sub1_nb.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/pub0sub1_nb.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/pub0sub1_nb.dir/flags.make

CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/pub0sub1_nb.c.o: CMakeFiles/pub0sub1_nb.dir/flags.make
CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/pub0sub1_nb.c.o: ../MQTTPacket/samples/pub0sub1_nb.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pi/paho.mqtt.embedded-c/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/pub0sub1_nb.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/pub0sub1_nb.c.o   -c /home/pi/paho.mqtt.embedded-c/MQTTPacket/samples/pub0sub1_nb.c

CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/pub0sub1_nb.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/pub0sub1_nb.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/pi/paho.mqtt.embedded-c/MQTTPacket/samples/pub0sub1_nb.c > CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/pub0sub1_nb.c.i

CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/pub0sub1_nb.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/pub0sub1_nb.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/pi/paho.mqtt.embedded-c/MQTTPacket/samples/pub0sub1_nb.c -o CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/pub0sub1_nb.c.s

CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/pub0sub1_nb.c.o.requires:

.PHONY : CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/pub0sub1_nb.c.o.requires

CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/pub0sub1_nb.c.o.provides: CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/pub0sub1_nb.c.o.requires
	$(MAKE) -f CMakeFiles/pub0sub1_nb.dir/build.make CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/pub0sub1_nb.c.o.provides.build
.PHONY : CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/pub0sub1_nb.c.o.provides

CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/pub0sub1_nb.c.o.provides.build: CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/pub0sub1_nb.c.o


CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/transport.c.o: CMakeFiles/pub0sub1_nb.dir/flags.make
CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/transport.c.o: ../MQTTPacket/samples/transport.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pi/paho.mqtt.embedded-c/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/transport.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/transport.c.o   -c /home/pi/paho.mqtt.embedded-c/MQTTPacket/samples/transport.c

CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/transport.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/transport.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/pi/paho.mqtt.embedded-c/MQTTPacket/samples/transport.c > CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/transport.c.i

CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/transport.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/transport.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/pi/paho.mqtt.embedded-c/MQTTPacket/samples/transport.c -o CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/transport.c.s

CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/transport.c.o.requires:

.PHONY : CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/transport.c.o.requires

CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/transport.c.o.provides: CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/transport.c.o.requires
	$(MAKE) -f CMakeFiles/pub0sub1_nb.dir/build.make CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/transport.c.o.provides.build
.PHONY : CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/transport.c.o.provides

CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/transport.c.o.provides.build: CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/transport.c.o


# Object files for target pub0sub1_nb
pub0sub1_nb_OBJECTS = \
"CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/pub0sub1_nb.c.o" \
"CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/transport.c.o"

# External object files for target pub0sub1_nb
pub0sub1_nb_EXTERNAL_OBJECTS =

pub0sub1_nb: CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/pub0sub1_nb.c.o
pub0sub1_nb: CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/transport.c.o
pub0sub1_nb: CMakeFiles/pub0sub1_nb.dir/build.make
pub0sub1_nb: libpaho-embed-mqtt3c.so
pub0sub1_nb: CMakeFiles/pub0sub1_nb.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/pi/paho.mqtt.embedded-c/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C executable pub0sub1_nb"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/pub0sub1_nb.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/pub0sub1_nb.dir/build: pub0sub1_nb

.PHONY : CMakeFiles/pub0sub1_nb.dir/build

CMakeFiles/pub0sub1_nb.dir/requires: CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/pub0sub1_nb.c.o.requires
CMakeFiles/pub0sub1_nb.dir/requires: CMakeFiles/pub0sub1_nb.dir/MQTTPacket/samples/transport.c.o.requires

.PHONY : CMakeFiles/pub0sub1_nb.dir/requires

CMakeFiles/pub0sub1_nb.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/pub0sub1_nb.dir/cmake_clean.cmake
.PHONY : CMakeFiles/pub0sub1_nb.dir/clean

CMakeFiles/pub0sub1_nb.dir/depend:
	cd /home/pi/paho.mqtt.embedded-c/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/pi/paho.mqtt.embedded-c /home/pi/paho.mqtt.embedded-c /home/pi/paho.mqtt.embedded-c/build /home/pi/paho.mqtt.embedded-c/build /home/pi/paho.mqtt.embedded-c/build/CMakeFiles/pub0sub1_nb.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/pub0sub1_nb.dir/depend
