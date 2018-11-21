set(MODULE_NAME Foundation)

set(CMAKE_CXX_STANDARD 11)

file(GLOB FOUNDATION_PRIVATE_SOURCES Private/*.h Private/*.cpp)
file(GLOB FOUNDATION_PUBLIC_SOURCES Public/*.h Public/*.inc)
source_group(Private FILES ${FOUNDATION_PRIVATE_SOURCES})
source_group(Public FILES ${FOUNDATION_PUBLIC_SOURCES})

add_library(${MODULE_NAME} ${FOUNDATION_PRIVATE_SOURCES} ${FOUNDATION_PUBLIC_SOURCES})

include(GenerateExportHeader)
target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
generate_export_header(${MODULE_NAME} EXPORT_MACRO_NAME FOUNDATION_API EXPORT_FILE_NAME FoundationAPI.h)

set_property(TARGET ${MODULE_NAME} PROPERTY CXX_STANDARD 14)
if(APPLE)
    set_property(SOURCE Private/PathTools.cpp PROPERTY COMPILE_FLAGS "-x objective-c++")
    find_library(FOUNDATION_LIBRARIES Foundation)
    target_link_libraries(${MODULE_NAME} PRIVATE ${FOUNDATION_LIBRARIES})
endif()
target_include_directories(${MODULE_NAME} PRIVATE Private)
target_include_directories(${MODULE_NAME} PUBLIC Public)
target_compile_options(${MODULE_NAME} ${DEFAULT_COMPILE_OPTIONS})
