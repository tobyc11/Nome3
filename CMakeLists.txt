cmake_minimum_required(VERSION 3.12)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Modules")

project(Nome3)

include(CompileOptions)

# Include sub-projects.
add_subdirectory(imgui)
add_subdirectory(glad)
add_subdirectory(Foundation)
add_subdirectory(Math)
add_subdirectory(Application)
