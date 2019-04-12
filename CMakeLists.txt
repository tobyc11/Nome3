cmake_minimum_required(VERSION 3.12)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Modules")

project(Nome3)

include(CompileOptions)

# Since ImGui is a submodule directly pulled from upstream, add its target here
add_library(imgui
	imgui/imgui.cpp
	imgui/imgui_demo.cpp
	imgui/imgui_draw.cpp
	imgui/imgui_widgets.cpp
)
target_include_directories(imgui INTERFACE imgui)
target_compile_options(imgui ${DEFAULT_COMPILE_OPTIONS})

# Include sub-projects.
add_subdirectory(Foundation)
add_subdirectory(Math)
add_subdirectory(Application)
