set(RENDERLIB_SRC
    Private/Buffer.cpp
    Private/BufferGeometry.cpp
    Private/CGLThread.cpp
    Private/CommandBuffer.cpp
    Private/DrawList.cpp
    Private/GridObject.cpp
    Private/GridObject.h
    Private/Material.cpp
    Private/MaterialEnumsGL.cpp
    Private/PipelineLibrary.cpp
    Private/RenderContext.cpp
    Private/ShaderManager.cpp
    Private/Shaders.cpp
    Private/glad/src/glad.c
    Public/Buffer.h
    Public/BufferGeometry.h
    Public/CGLThread.h
    Public/CommandBuffer.h
    Public/DrawList.h
    Public/Material.h
    Public/MaterialEnums.h
    Public/MaterialEnumsGL.h
    Public/PipelineLibrary.h
    Public/RenderContext.h
    Public/ShaderManager.h
    Public/Shaders.h
    Public/TimelineQueue.h
)

add_library(CADRender ${RENDERLIB_SRC})
target_include_directories(CADRender PRIVATE ./Private)
target_include_directories(CADRender PRIVATE ./Private/glad/include)
target_include_directories(CADRender PUBLIC ./Public)

target_link_libraries(CADRender PRIVATE Math)
