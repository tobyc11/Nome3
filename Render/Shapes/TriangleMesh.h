#pragma once

#include "Shape.h"

#include <Device.h>
#include <DrawTemplate.h>

#include <map>

namespace Nome::Render
{

//Test mesh
class CTestConstMeshShape : public CShape
{
public:
    tc::BoundingBox ObjectBound() const override;
    void ChooseShaders(CShaderCombiner& combiner) const override;
    void BindPipelineArgs(RHI::CDrawTemplate& drawTemplate) const override;
};

enum class EMeshAttribute
{
    Position,
    Normal,
    Tangent,
    TexCoord0,
    TexCoord1,
    Color0,
    Joints0,
    Weights0
};

//A mesh or a subset of a mesh that is drawable in a single draw call
//Only deals with GPU data
class CBufferMeshShape : public CShape
{
public:
    tc::BoundingBox ObjectBound() const override;
    void ChooseShaders(CShaderCombiner& combiner) const override;
    void BindPipelineArgs(RHI::CDrawTemplate& drawTemplate) const override;

    void SetAttribute(EMeshAttribute attr, const RHI::CBufferView& bufferView, uint32_t offset);
    void SetObjectBound(const tc::BoundingBox& value);
    void SetElementCount(uint32_t v) { ElementCount = v; }
    void SetIndexBuffer(const RHI::CBufferView& bufferView, uint32_t offset, uint32_t count);
    void SetPrimitiveTopology(RHI::EPrimitiveTopology t) { Topology = t; }

private:
    tc::BoundingBox Bounds;

    std::vector<RHI::CVertexInputAttributeDesc> VertexAttributeDescs;
    std::vector<RHI::CVertexInputBindingDesc> VertexBindingDescs;
    RHI::CVertexInputs InputBindings;

    uint32_t ElementCount = 0;

    tc::sp<RHI::CBuffer> IndexBuffer;
    uint32_t IndexByteOffset;
    RHI::EFormat IndexFormat;

    RHI::EPrimitiveTopology Topology;

    bool bHasTangent = false;
    bool bHasUV0 = false;
};

} // namespace Nome::Render
