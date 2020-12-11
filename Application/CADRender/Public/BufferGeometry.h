#pragma once
#include "Buffer.h"
#include "CommandBuffer.h"

class CBufferGeometry
{
public:
    CBufferGeometry(std::vector<CBufferDesc> buffers, std::vector<CAttributeDesc> attrs, EPrimitiveTopology topology,
                    std::shared_ptr<CBuffer> indexBuffer = nullptr, EBaseType indexType = EBaseType::Invalid)
        : Buffers(std::move(buffers))
        , Attributes(std::move(attrs))
        , Topology(topology)
        , IndexBuffer(std::move(indexBuffer))
        , IndexType(indexType)
    {
    }

    // Secondary constructors
    template <int sz1, int sz2>
    CBufferGeometry(CBufferDesc (&b)[sz1], CAttributeDesc (&a)[sz2], EPrimitiveTopology topology,
                    std::shared_ptr<CBuffer> indexBuffer = nullptr, EBaseType indexType = EBaseType::Invalid)
        : Buffers(b, b + sz1)
        , Attributes(a, a + sz2)
        , Topology(topology)
        , IndexBuffer(std::move(indexBuffer))
        , IndexType(indexType)
    {
    }

    [[nodiscard]] const std::vector<CBufferDesc>& GetBuffers() const { return Buffers; }
    [[nodiscard]] std::vector<CBufferBindingDesc> GetBufferBindings() const;
    [[nodiscard]] const std::vector<CAttributeDesc>& GetAttributes() const { return Attributes; }
    [[nodiscard]] EPrimitiveTopology GetTopology() const { return Topology; }
    [[nodiscard]] const std::shared_ptr<CBuffer>& GetIndexBuffer() const { return IndexBuffer; }
    [[nodiscard]] EBaseType GetIndexType() const { return IndexType; }

    void BindVertexBuffers(CCommandBuffer& cmdBuffer) const;

private:
    std::vector<CBufferDesc> Buffers;
    std::vector<CAttributeDesc> Attributes;
    EPrimitiveTopology Topology;
    std::shared_ptr<CBuffer> IndexBuffer;
    EBaseType IndexType;
};
