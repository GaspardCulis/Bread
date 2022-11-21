cmake_minimum_required(VERSION 3.10)

project(Bread)

# Here we can set some values for botcraft options
# They will still appear in cmake_gui in windows
set(BOTCRAFT_BUILD_EXAMPLES OFF CACHE BOOL "")

# This will enter the Botcraft folder and
# process Botcrat/CMakeLists.txt to add its
# targets to our current project
add_subdirectory(Botcraft)

# This will enter our My_Code folder and
# process My_Code/CMakeLists.txt
add_subdirectory(Bread)