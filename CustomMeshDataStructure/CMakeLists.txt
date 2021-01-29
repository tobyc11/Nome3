set(MODULE_NAME CustomMeshDataStructures)

set(CMAKE_CXX_STANDARD 11)

file(GLOB CPP_SOURCES *.cpp)
file(GLOB H_SOURCES *.h)

add_library(${MODULE_NAME} ${CPP_SOURCES} ${H_SOURCES})

#Generate export header
include(GenerateExportHeader)
target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
generate_export_header(${MODULE_NAME} EXPORT_FILE_NAME CustomMeshDataStructureAPI.h)

target_include_directories(${MODULE_NAME} PUBLIC .)
target_compile_definitions(${MODULE_NAME} PUBLIC URHO3D_SSE)


macro(config_playground2 target)
    target_link_libraries(${target} PRIVATE Math)
    target_compile_options(${target} ${DEFAULT_COMPILE_OPTIONS})
endmacro()

if(MSVC)
	target_compile_definitions(${MODULE_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

if(DEFAULT_COMPILE_OPTIONS)
    target_compile_options(${MODULE_NAME} ${DEFAULT_COMPILE_OPTIONS})
endif()
