# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.26

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
CMAKE_COMMAND = /opt/homebrew/Cellar/cmake/3.26.4/bin/cmake

# The command to remove a file.
RM = /opt/homebrew/Cellar/cmake/3.26.4/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/build

# Include any dependencies generated for this target.
include vulkan/CMakeFiles/vulkan.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include vulkan/CMakeFiles/vulkan.dir/compiler_depend.make

# Include the progress variables for this target.
include vulkan/CMakeFiles/vulkan.dir/progress.make

# Include the compile flags for this target's objects.
include vulkan/CMakeFiles/vulkan.dir/flags.make

vulkan/CMakeFiles/vulkan.dir/Device.cpp.o: vulkan/CMakeFiles/vulkan.dir/flags.make
vulkan/CMakeFiles/vulkan.dir/Device.cpp.o: /Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/vulkan/Device.cpp
vulkan/CMakeFiles/vulkan.dir/Device.cpp.o: vulkan/CMakeFiles/vulkan.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object vulkan/CMakeFiles/vulkan.dir/Device.cpp.o"
	cd /Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/build/vulkan && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT vulkan/CMakeFiles/vulkan.dir/Device.cpp.o -MF CMakeFiles/vulkan.dir/Device.cpp.o.d -o CMakeFiles/vulkan.dir/Device.cpp.o -c /Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/vulkan/Device.cpp

vulkan/CMakeFiles/vulkan.dir/Device.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/vulkan.dir/Device.cpp.i"
	cd /Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/build/vulkan && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/vulkan/Device.cpp > CMakeFiles/vulkan.dir/Device.cpp.i

vulkan/CMakeFiles/vulkan.dir/Device.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/vulkan.dir/Device.cpp.s"
	cd /Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/build/vulkan && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/vulkan/Device.cpp -o CMakeFiles/vulkan.dir/Device.cpp.s

vulkan/CMakeFiles/vulkan.dir/Instance.cpp.o: vulkan/CMakeFiles/vulkan.dir/flags.make
vulkan/CMakeFiles/vulkan.dir/Instance.cpp.o: /Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/vulkan/Instance.cpp
vulkan/CMakeFiles/vulkan.dir/Instance.cpp.o: vulkan/CMakeFiles/vulkan.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object vulkan/CMakeFiles/vulkan.dir/Instance.cpp.o"
	cd /Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/build/vulkan && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT vulkan/CMakeFiles/vulkan.dir/Instance.cpp.o -MF CMakeFiles/vulkan.dir/Instance.cpp.o.d -o CMakeFiles/vulkan.dir/Instance.cpp.o -c /Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/vulkan/Instance.cpp

vulkan/CMakeFiles/vulkan.dir/Instance.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/vulkan.dir/Instance.cpp.i"
	cd /Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/build/vulkan && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/vulkan/Instance.cpp > CMakeFiles/vulkan.dir/Instance.cpp.i

vulkan/CMakeFiles/vulkan.dir/Instance.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/vulkan.dir/Instance.cpp.s"
	cd /Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/build/vulkan && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/vulkan/Instance.cpp -o CMakeFiles/vulkan.dir/Instance.cpp.s

vulkan/CMakeFiles/vulkan.dir/VkContext.cpp.o: vulkan/CMakeFiles/vulkan.dir/flags.make
vulkan/CMakeFiles/vulkan.dir/VkContext.cpp.o: /Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/vulkan/VkContext.cpp
vulkan/CMakeFiles/vulkan.dir/VkContext.cpp.o: vulkan/CMakeFiles/vulkan.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object vulkan/CMakeFiles/vulkan.dir/VkContext.cpp.o"
	cd /Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/build/vulkan && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT vulkan/CMakeFiles/vulkan.dir/VkContext.cpp.o -MF CMakeFiles/vulkan.dir/VkContext.cpp.o.d -o CMakeFiles/vulkan.dir/VkContext.cpp.o -c /Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/vulkan/VkContext.cpp

vulkan/CMakeFiles/vulkan.dir/VkContext.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/vulkan.dir/VkContext.cpp.i"
	cd /Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/build/vulkan && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/vulkan/VkContext.cpp > CMakeFiles/vulkan.dir/VkContext.cpp.i

vulkan/CMakeFiles/vulkan.dir/VkContext.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/vulkan.dir/VkContext.cpp.s"
	cd /Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/build/vulkan && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/vulkan/VkContext.cpp -o CMakeFiles/vulkan.dir/VkContext.cpp.s

# Object files for target vulkan
vulkan_OBJECTS = \
"CMakeFiles/vulkan.dir/Device.cpp.o" \
"CMakeFiles/vulkan.dir/Instance.cpp.o" \
"CMakeFiles/vulkan.dir/VkContext.cpp.o"

# External object files for target vulkan
vulkan_EXTERNAL_OBJECTS =

/Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/library/libvulkan.a: vulkan/CMakeFiles/vulkan.dir/Device.cpp.o
/Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/library/libvulkan.a: vulkan/CMakeFiles/vulkan.dir/Instance.cpp.o
/Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/library/libvulkan.a: vulkan/CMakeFiles/vulkan.dir/VkContext.cpp.o
/Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/library/libvulkan.a: vulkan/CMakeFiles/vulkan.dir/build.make
/Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/library/libvulkan.a: vulkan/CMakeFiles/vulkan.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX static library /Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/library/libvulkan.a"
	cd /Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/build/vulkan && $(CMAKE_COMMAND) -P CMakeFiles/vulkan.dir/cmake_clean_target.cmake
	cd /Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/build/vulkan && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/vulkan.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
vulkan/CMakeFiles/vulkan.dir/build: /Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/library/libvulkan.a
.PHONY : vulkan/CMakeFiles/vulkan.dir/build

vulkan/CMakeFiles/vulkan.dir/clean:
	cd /Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/build/vulkan && $(CMAKE_COMMAND) -P CMakeFiles/vulkan.dir/cmake_clean.cmake
.PHONY : vulkan/CMakeFiles/vulkan.dir/clean

vulkan/CMakeFiles/vulkan.dir/depend:
	cd /Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine /Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/vulkan /Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/build /Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/build/vulkan /Users/uncled/Documents/CFiles/VulkanEngine/RenderEngine/build/vulkan/CMakeFiles/vulkan.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : vulkan/CMakeFiles/vulkan.dir/depend

