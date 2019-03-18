set(MODULE_NAME Render)

set(CMAKE_CXX_STANDARD 17)

file(GLOB RENDER_CORE_SOURCES Core/*.h Core/*.cpp)
file(GLOB RENDER_MATERIALS_SOURCES Materials/*.h Materials/*.cpp)
file(GLOB RENDER_SHAPES_SOURCES Shapes/*.h Shapes/*.cpp)
source_group(Core FILES ${RENDER_CORE_SOURCES})
source_group(Materials FILES ${RENDER_MATERIALS_SOURCES})
source_group(Shapes FILES ${RENDER_SHAPES_SOURCES})

add_library(${MODULE_NAME} ${RENDER_CORE_SOURCES} ${RENDER_MATERIALS_SOURCES} ${RENDER_SHAPES_SOURCES})

include(GenerateExportHeader)
target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
generate_export_header(${MODULE_NAME} EXPORT_MACRO_NAME RENDER_API EXPORT_FILE_NAME RenderLinkage.h)

target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(${MODULE_NAME} ${DEFAULT_COMPILE_OPTIONS})

target_link_libraries(${MODULE_NAME} PUBLIC RHI)

find_package(Slang REQUIRED)
target_link_libraries(${MODULE_NAME} PRIVATE Slang::Slang)
