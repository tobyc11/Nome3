#pragma once
#include "MaterialEnums.h"
#include "RenderContext.h"
#include <cstdlib>
#include <memory>
#include <vector>

class CGLThread;

// Nome 3 public API enums

enum class EBufferType
{
    Vertex,
    Index,
    Uniform
};

enum class EVertexAttribute
{
    Position,
    Normal,
    Index0,
    Index1,
    Index2
};

enum class EBaseType
{
    Int8 = 0,
    UInt8,
    Int16,
    UInt16,
    Int32,
    UInt32,
    Float,
    Double,
    Invalid
};

// This handle is considered internal data
class CBufferHandle
{
public:
    [[nodiscard]] bool IsValid() const { return BufferId != 0; }

    unsigned int BufferId {};
    int BindTarget {};
};

class CBuffer : public CRenderContextChild
{
public:
    CBuffer(EBufferType type, size_t size, const void* initialData = nullptr);

    CBuffer(const CBuffer& other) = delete;
    CBuffer& operator=(const CBuffer& other) = delete;

    ~CBuffer();

    [[nodiscard]] const CBufferHandle& InternalGetBufferHandle() const { return BufferHandle; }

private:
    CBufferHandle BufferHandle;
};

class CStagingBuffer
{
public:
    CStagingBuffer(EBufferType type, uint32_t size);

    // Get the address of this staging buffer
    void* GetPtr();
    // Finish staging and convert into a GPU buffer
    std::shared_ptr<CBuffer> Finalize();

private:
    struct CFreeDeleter
    {
        void operator()(void* x) { free(x); }
    };

    std::unique_ptr<void, CFreeDeleter> BackingBuffer;
};

struct CBufferBindingDesc
{
    // Binding is the index into the vertex buffer array bound by CCommandBuffer::BindVertexBuffers
    uint32_t Binding;
    uint32_t Stride;
    // What is InputRate?
    // InstanceId / InputRate = Index into this buffer, 0 designates perVertex
    uint32_t InputRate;
};

struct CBufferDesc
{
    // Note: Buffer and Offset are bound inside command buffer, binding is implicit in this case
    std::shared_ptr<CBuffer> Buffer;
    uint32_t Offset;
    // Note: Stride and InputRate are part of pipeline states
    uint32_t Stride;
    // What is InputRate?
    // InstanceId / InputRate = Index into this buffer, 0 designates perVertex
    uint32_t InputRate;
};

struct CAttributeDesc
{
    EVertexAttribute Attribute;
    uint32_t Binding;
    EBaseType BaseType;
    uint32_t Components;
    uint32_t Offset;
};
