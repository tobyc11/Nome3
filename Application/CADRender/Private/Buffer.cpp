#include "Buffer.h"
#include "CGLThread.h"
#include <glad/glad.h>

CBuffer::CBuffer(EBufferType type, size_t size, const void* initialData)
{
    int purpose = GL_ARRAY_BUFFER;
    if (type == EBufferType::Index)
        purpose = GL_ELEMENT_ARRAY_BUFFER;
    else if (type == EBufferType::Uniform)
        purpose = GL_UNIFORM_BUFFER;
    BufferHandle = ParentCtx->GetDevice().MakeBuffer(purpose, size, initialData);
}

CBuffer::~CBuffer() { ParentCtx->GetDevice().DestroyBuffer(BufferHandle); }

CStagingBuffer::CStagingBuffer(EBufferType type, uint32_t size)
{
    // On UMA, we create a buffer from device API directly if the API supports it
    // On discrete GPU, we create staging buffer from API
    // On OpenGL which does not support concurrency, just create a CPU buffer
    BackingBuffer.reset(malloc(size));
}

void* CStagingBuffer::GetPtr() { return BackingBuffer.get(); }

std::shared_ptr<CBuffer> CStagingBuffer::Finalize() { return nullptr; }
