#include "TriangleMesh.h"

#include <RHIInstance.h>

namespace Nome::Render
{

tc::BoundingBox CTestConstMeshShape::ObjectBound() const
{
    return tc::BoundingBox(-1.0f, 1.0f);
}

void CTestConstMeshShape::ChooseShaders(CShaderCombiner& combiner) const
{
    combiner.SetVertexShader("TestConstMesh");
    combiner.SetSurfaceInteraction("TestConstMeshVSOut");
}

void CTestConstMeshShape::BindPipelineArgs(
    RHI::CDrawTemplate& drawTemplate) const
{
    drawTemplate.ElementCount = 3;
    drawTemplate.InstanceCount = 0;
    drawTemplate.VertexOffset = 0;
    drawTemplate.IndexOffset = 0;
    drawTemplate.InstanceOffset = 0;
}

tc::BoundingBox CBufferMeshShape::ObjectBound() const
{
    return Bounds;
}

void CBufferMeshShape::ChooseShaders(CShaderCombiner& combiner) const
{
    combiner.SetVertexShader("StaticMesh");
    combiner.SetSurfaceInteraction("StaticMeshVSOut");
    if (bHasTangent)
        combiner.AddCompileDefinition("HAS_TANGENT");
    if (bHasUV0)
        combiner.AddCompileDefinition("HAS_UV0");
}

void CBufferMeshShape::BindPipelineArgs(RHI::CDrawTemplate& drawTemplate) const
{
    drawTemplate.GetVertexAttributeDescs() = VertexAttributeDescs;
    drawTemplate.GetVertexBindingDescs() = VertexBindingDescs;
    drawTemplate.GetVertexInputs() = InputBindings;

    if (IndexBuffer)
    {
        drawTemplate.SetIndexBuffer(IndexBuffer, IndexByteOffset, IndexFormat);
    }
    drawTemplate.ElementCount = ElementCount;
    drawTemplate.InstanceCount = 0;
    drawTemplate.VertexOffset = 0;
    drawTemplate.IndexOffset = 0;
    drawTemplate.InstanceOffset = 0;
    drawTemplate.SetPrimitiveTopology(Topology);
}

void CBufferMeshShape::SetAttribute(EMeshAttribute attr, const RHI::CBufferView& bufferView, uint32_t offset)
{
    using F = RHI::EFormat;
    switch (attr)
    {
    case Nome::Render::EMeshAttribute::Position:
        VertexAttributeDescs.push_back(RHI::CVertexInputAttributeDesc{ 0, F::R32G32B32_SFLOAT, 0, 0 });
        VertexBindingDescs.push_back(RHI::CVertexInputBindingDesc{ 0, bufferView.Stride, false });
        InputBindings.AddAccessor(0, bufferView.Buffer, bufferView.Offset + offset);
        break;
    case Nome::Render::EMeshAttribute::Normal:
        VertexAttributeDescs.push_back(RHI::CVertexInputAttributeDesc{ 1, F::R32G32B32_SFLOAT, 0, 1 });
        VertexBindingDescs.push_back(RHI::CVertexInputBindingDesc{ 1, bufferView.Stride, false });
        InputBindings.AddAccessor(1, bufferView.Buffer, bufferView.Offset + offset);
        break;
    case Nome::Render::EMeshAttribute::Tangent:
        bHasTangent = true;
        VertexAttributeDescs.push_back(RHI::CVertexInputAttributeDesc{ 2, F::R32G32B32A32_SFLOAT, 0, 2 });
        VertexBindingDescs.push_back(RHI::CVertexInputBindingDesc{ 2, bufferView.Stride, false });
        InputBindings.AddAccessor(2, bufferView.Buffer, bufferView.Offset + offset);
        break;
    case Nome::Render::EMeshAttribute::TexCoord0:
        bHasUV0 = true;
        VertexAttributeDescs.push_back(RHI::CVertexInputAttributeDesc{ 3, F::R32G32_SFLOAT, 0, 3 });
        VertexBindingDescs.push_back(RHI::CVertexInputBindingDesc{ 3, bufferView.Stride, false });
        InputBindings.AddAccessor(3, bufferView.Buffer, bufferView.Offset + offset);
        break;
    case Nome::Render::EMeshAttribute::TexCoord1:
    case Nome::Render::EMeshAttribute::Color0:
    case Nome::Render::EMeshAttribute::Joints0:
    case Nome::Render::EMeshAttribute::Weights0:
    default:
        throw "unimplemented";
        break;
    }
}

void CBufferMeshShape::SetObjectBound(const tc::BoundingBox& value)
{
    Bounds = value;
}

void CBufferMeshShape::SetIndexBuffer(const RHI::CBufferView& bufferView, uint32_t offset, uint32_t count)
{
    IndexBuffer = bufferView.Buffer;
    IndexByteOffset = bufferView.Offset + offset;
    ElementCount = count;
    if (bufferView.Stride == 2)
        IndexFormat = RHI::EFormat::R16_UINT;
    else
        IndexFormat = RHI::EFormat::R32_UINT;
}

} // namespace Nome::Render
