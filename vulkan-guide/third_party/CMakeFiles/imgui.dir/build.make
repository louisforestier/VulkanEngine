# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.24

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
CMAKE_COMMAND = /snap/cmake/1156/bin/cmake

# The command to remove a file.
RM = /snap/cmake/1156/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/louisf/Documents/VulkanProject/vulkan-guide

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/louisf/Documents/VulkanProject/vulkan-guide

# Include any dependencies generated for this target.
include third_party/CMakeFiles/imgui.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include third_party/CMakeFiles/imgui.dir/compiler_depend.make

# Include the progress variables for this target.
include third_party/CMakeFiles/imgui.dir/progress.make

# Include the compile flags for this target's objects.
include third_party/CMakeFiles/imgui.dir/flags.make

third_party/CMakeFiles/imgui.dir/imgui/imgui.cpp.o: third_party/CMakeFiles/imgui.dir/flags.make
third_party/CMakeFiles/imgui.dir/imgui/imgui.cpp.o: third_party/imgui/imgui.cpp
third_party/CMakeFiles/imgui.dir/imgui/imgui.cpp.o: third_party/CMakeFiles/imgui.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/louisf/Documents/VulkanProject/vulkan-guide/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object third_party/CMakeFiles/imgui.dir/imgui/imgui.cpp.o"
	cd /home/louisf/Documents/VulkanProject/vulkan-guide/third_party && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT third_party/CMakeFiles/imgui.dir/imgui/imgui.cpp.o -MF CMakeFiles/imgui.dir/imgui/imgui.cpp.o.d -o CMakeFiles/imgui.dir/imgui/imgui.cpp.o -c /home/louisf/Documents/VulkanProject/vulkan-guide/third_party/imgui/imgui.cpp

third_party/CMakeFiles/imgui.dir/imgui/imgui.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/imgui.dir/imgui/imgui.cpp.i"
	cd /home/louisf/Documents/VulkanProject/vulkan-guide/third_party && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/louisf/Documents/VulkanProject/vulkan-guide/third_party/imgui/imgui.cpp > CMakeFiles/imgui.dir/imgui/imgui.cpp.i

third_party/CMakeFiles/imgui.dir/imgui/imgui.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/imgui.dir/imgui/imgui.cpp.s"
	cd /home/louisf/Documents/VulkanProject/vulkan-guide/third_party && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/louisf/Documents/VulkanProject/vulkan-guide/third_party/imgui/imgui.cpp -o CMakeFiles/imgui.dir/imgui/imgui.cpp.s

third_party/CMakeFiles/imgui.dir/imgui/imgui_demo.cpp.o: third_party/CMakeFiles/imgui.dir/flags.make
third_party/CMakeFiles/imgui.dir/imgui/imgui_demo.cpp.o: third_party/imgui/imgui_demo.cpp
third_party/CMakeFiles/imgui.dir/imgui/imgui_demo.cpp.o: third_party/CMakeFiles/imgui.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/louisf/Documents/VulkanProject/vulkan-guide/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object third_party/CMakeFiles/imgui.dir/imgui/imgui_demo.cpp.o"
	cd /home/louisf/Documents/VulkanProject/vulkan-guide/third_party && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT third_party/CMakeFiles/imgui.dir/imgui/imgui_demo.cpp.o -MF CMakeFiles/imgui.dir/imgui/imgui_demo.cpp.o.d -o CMakeFiles/imgui.dir/imgui/imgui_demo.cpp.o -c /home/louisf/Documents/VulkanProject/vulkan-guide/third_party/imgui/imgui_demo.cpp

third_party/CMakeFiles/imgui.dir/imgui/imgui_demo.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/imgui.dir/imgui/imgui_demo.cpp.i"
	cd /home/louisf/Documents/VulkanProject/vulkan-guide/third_party && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/louisf/Documents/VulkanProject/vulkan-guide/third_party/imgui/imgui_demo.cpp > CMakeFiles/imgui.dir/imgui/imgui_demo.cpp.i

third_party/CMakeFiles/imgui.dir/imgui/imgui_demo.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/imgui.dir/imgui/imgui_demo.cpp.s"
	cd /home/louisf/Documents/VulkanProject/vulkan-guide/third_party && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/louisf/Documents/VulkanProject/vulkan-guide/third_party/imgui/imgui_demo.cpp -o CMakeFiles/imgui.dir/imgui/imgui_demo.cpp.s

third_party/CMakeFiles/imgui.dir/imgui/imgui_draw.cpp.o: third_party/CMakeFiles/imgui.dir/flags.make
third_party/CMakeFiles/imgui.dir/imgui/imgui_draw.cpp.o: third_party/imgui/imgui_draw.cpp
third_party/CMakeFiles/imgui.dir/imgui/imgui_draw.cpp.o: third_party/CMakeFiles/imgui.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/louisf/Documents/VulkanProject/vulkan-guide/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object third_party/CMakeFiles/imgui.dir/imgui/imgui_draw.cpp.o"
	cd /home/louisf/Documents/VulkanProject/vulkan-guide/third_party && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT third_party/CMakeFiles/imgui.dir/imgui/imgui_draw.cpp.o -MF CMakeFiles/imgui.dir/imgui/imgui_draw.cpp.o.d -o CMakeFiles/imgui.dir/imgui/imgui_draw.cpp.o -c /home/louisf/Documents/VulkanProject/vulkan-guide/third_party/imgui/imgui_draw.cpp

third_party/CMakeFiles/imgui.dir/imgui/imgui_draw.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/imgui.dir/imgui/imgui_draw.cpp.i"
	cd /home/louisf/Documents/VulkanProject/vulkan-guide/third_party && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/louisf/Documents/VulkanProject/vulkan-guide/third_party/imgui/imgui_draw.cpp > CMakeFiles/imgui.dir/imgui/imgui_draw.cpp.i

third_party/CMakeFiles/imgui.dir/imgui/imgui_draw.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/imgui.dir/imgui/imgui_draw.cpp.s"
	cd /home/louisf/Documents/VulkanProject/vulkan-guide/third_party && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/louisf/Documents/VulkanProject/vulkan-guide/third_party/imgui/imgui_draw.cpp -o CMakeFiles/imgui.dir/imgui/imgui_draw.cpp.s

third_party/CMakeFiles/imgui.dir/imgui/imgui_widgets.cpp.o: third_party/CMakeFiles/imgui.dir/flags.make
third_party/CMakeFiles/imgui.dir/imgui/imgui_widgets.cpp.o: third_party/imgui/imgui_widgets.cpp
third_party/CMakeFiles/imgui.dir/imgui/imgui_widgets.cpp.o: third_party/CMakeFiles/imgui.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/louisf/Documents/VulkanProject/vulkan-guide/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object third_party/CMakeFiles/imgui.dir/imgui/imgui_widgets.cpp.o"
	cd /home/louisf/Documents/VulkanProject/vulkan-guide/third_party && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT third_party/CMakeFiles/imgui.dir/imgui/imgui_widgets.cpp.o -MF CMakeFiles/imgui.dir/imgui/imgui_widgets.cpp.o.d -o CMakeFiles/imgui.dir/imgui/imgui_widgets.cpp.o -c /home/louisf/Documents/VulkanProject/vulkan-guide/third_party/imgui/imgui_widgets.cpp

third_party/CMakeFiles/imgui.dir/imgui/imgui_widgets.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/imgui.dir/imgui/imgui_widgets.cpp.i"
	cd /home/louisf/Documents/VulkanProject/vulkan-guide/third_party && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/louisf/Documents/VulkanProject/vulkan-guide/third_party/imgui/imgui_widgets.cpp > CMakeFiles/imgui.dir/imgui/imgui_widgets.cpp.i

third_party/CMakeFiles/imgui.dir/imgui/imgui_widgets.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/imgui.dir/imgui/imgui_widgets.cpp.s"
	cd /home/louisf/Documents/VulkanProject/vulkan-guide/third_party && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/louisf/Documents/VulkanProject/vulkan-guide/third_party/imgui/imgui_widgets.cpp -o CMakeFiles/imgui.dir/imgui/imgui_widgets.cpp.s

third_party/CMakeFiles/imgui.dir/imgui/imgui_impl_vulkan.cpp.o: third_party/CMakeFiles/imgui.dir/flags.make
third_party/CMakeFiles/imgui.dir/imgui/imgui_impl_vulkan.cpp.o: third_party/imgui/imgui_impl_vulkan.cpp
third_party/CMakeFiles/imgui.dir/imgui/imgui_impl_vulkan.cpp.o: third_party/CMakeFiles/imgui.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/louisf/Documents/VulkanProject/vulkan-guide/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object third_party/CMakeFiles/imgui.dir/imgui/imgui_impl_vulkan.cpp.o"
	cd /home/louisf/Documents/VulkanProject/vulkan-guide/third_party && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT third_party/CMakeFiles/imgui.dir/imgui/imgui_impl_vulkan.cpp.o -MF CMakeFiles/imgui.dir/imgui/imgui_impl_vulkan.cpp.o.d -o CMakeFiles/imgui.dir/imgui/imgui_impl_vulkan.cpp.o -c /home/louisf/Documents/VulkanProject/vulkan-guide/third_party/imgui/imgui_impl_vulkan.cpp

third_party/CMakeFiles/imgui.dir/imgui/imgui_impl_vulkan.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/imgui.dir/imgui/imgui_impl_vulkan.cpp.i"
	cd /home/louisf/Documents/VulkanProject/vulkan-guide/third_party && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/louisf/Documents/VulkanProject/vulkan-guide/third_party/imgui/imgui_impl_vulkan.cpp > CMakeFiles/imgui.dir/imgui/imgui_impl_vulkan.cpp.i

third_party/CMakeFiles/imgui.dir/imgui/imgui_impl_vulkan.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/imgui.dir/imgui/imgui_impl_vulkan.cpp.s"
	cd /home/louisf/Documents/VulkanProject/vulkan-guide/third_party && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/louisf/Documents/VulkanProject/vulkan-guide/third_party/imgui/imgui_impl_vulkan.cpp -o CMakeFiles/imgui.dir/imgui/imgui_impl_vulkan.cpp.s

third_party/CMakeFiles/imgui.dir/imgui/imgui_impl_sdl.cpp.o: third_party/CMakeFiles/imgui.dir/flags.make
third_party/CMakeFiles/imgui.dir/imgui/imgui_impl_sdl.cpp.o: third_party/imgui/imgui_impl_sdl.cpp
third_party/CMakeFiles/imgui.dir/imgui/imgui_impl_sdl.cpp.o: third_party/CMakeFiles/imgui.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/louisf/Documents/VulkanProject/vulkan-guide/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object third_party/CMakeFiles/imgui.dir/imgui/imgui_impl_sdl.cpp.o"
	cd /home/louisf/Documents/VulkanProject/vulkan-guide/third_party && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT third_party/CMakeFiles/imgui.dir/imgui/imgui_impl_sdl.cpp.o -MF CMakeFiles/imgui.dir/imgui/imgui_impl_sdl.cpp.o.d -o CMakeFiles/imgui.dir/imgui/imgui_impl_sdl.cpp.o -c /home/louisf/Documents/VulkanProject/vulkan-guide/third_party/imgui/imgui_impl_sdl.cpp

third_party/CMakeFiles/imgui.dir/imgui/imgui_impl_sdl.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/imgui.dir/imgui/imgui_impl_sdl.cpp.i"
	cd /home/louisf/Documents/VulkanProject/vulkan-guide/third_party && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/louisf/Documents/VulkanProject/vulkan-guide/third_party/imgui/imgui_impl_sdl.cpp > CMakeFiles/imgui.dir/imgui/imgui_impl_sdl.cpp.i

third_party/CMakeFiles/imgui.dir/imgui/imgui_impl_sdl.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/imgui.dir/imgui/imgui_impl_sdl.cpp.s"
	cd /home/louisf/Documents/VulkanProject/vulkan-guide/third_party && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/louisf/Documents/VulkanProject/vulkan-guide/third_party/imgui/imgui_impl_sdl.cpp -o CMakeFiles/imgui.dir/imgui/imgui_impl_sdl.cpp.s

# Object files for target imgui
imgui_OBJECTS = \
"CMakeFiles/imgui.dir/imgui/imgui.cpp.o" \
"CMakeFiles/imgui.dir/imgui/imgui_demo.cpp.o" \
"CMakeFiles/imgui.dir/imgui/imgui_draw.cpp.o" \
"CMakeFiles/imgui.dir/imgui/imgui_widgets.cpp.o" \
"CMakeFiles/imgui.dir/imgui/imgui_impl_vulkan.cpp.o" \
"CMakeFiles/imgui.dir/imgui/imgui_impl_sdl.cpp.o"

# External object files for target imgui
imgui_EXTERNAL_OBJECTS =

third_party/libimgui.a: third_party/CMakeFiles/imgui.dir/imgui/imgui.cpp.o
third_party/libimgui.a: third_party/CMakeFiles/imgui.dir/imgui/imgui_demo.cpp.o
third_party/libimgui.a: third_party/CMakeFiles/imgui.dir/imgui/imgui_draw.cpp.o
third_party/libimgui.a: third_party/CMakeFiles/imgui.dir/imgui/imgui_widgets.cpp.o
third_party/libimgui.a: third_party/CMakeFiles/imgui.dir/imgui/imgui_impl_vulkan.cpp.o
third_party/libimgui.a: third_party/CMakeFiles/imgui.dir/imgui/imgui_impl_sdl.cpp.o
third_party/libimgui.a: third_party/CMakeFiles/imgui.dir/build.make
third_party/libimgui.a: third_party/CMakeFiles/imgui.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/louisf/Documents/VulkanProject/vulkan-guide/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Linking CXX static library libimgui.a"
	cd /home/louisf/Documents/VulkanProject/vulkan-guide/third_party && $(CMAKE_COMMAND) -P CMakeFiles/imgui.dir/cmake_clean_target.cmake
	cd /home/louisf/Documents/VulkanProject/vulkan-guide/third_party && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/imgui.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
third_party/CMakeFiles/imgui.dir/build: third_party/libimgui.a
.PHONY : third_party/CMakeFiles/imgui.dir/build

third_party/CMakeFiles/imgui.dir/clean:
	cd /home/louisf/Documents/VulkanProject/vulkan-guide/third_party && $(CMAKE_COMMAND) -P CMakeFiles/imgui.dir/cmake_clean.cmake
.PHONY : third_party/CMakeFiles/imgui.dir/clean

third_party/CMakeFiles/imgui.dir/depend:
	cd /home/louisf/Documents/VulkanProject/vulkan-guide && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/louisf/Documents/VulkanProject/vulkan-guide /home/louisf/Documents/VulkanProject/vulkan-guide/third_party /home/louisf/Documents/VulkanProject/vulkan-guide /home/louisf/Documents/VulkanProject/vulkan-guide/third_party /home/louisf/Documents/VulkanProject/vulkan-guide/third_party/CMakeFiles/imgui.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : third_party/CMakeFiles/imgui.dir/depend

