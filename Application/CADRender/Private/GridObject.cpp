#include "GridObject.h"
#include "ShaderManager.h"

CGridObject::CGridObject()
{
    MakeGeometry();
    MakeMaterial();
}

void CGridObject::RegenerateGeometry(int count, float spacing)
{
    LineSegmentCount = count;
    LineSpacing = spacing;
    MakeGeometry();
}

void CGridObject::Draw(CCommandBuffer& cmdBuffer, const CCameraParams& cameraParams) const
{
    // Set necessary states
    cmdBuffer.BindMaterial(Material);

    // Set the following variables in LineInstanced
    // uniform mat4 MMat;
    // uniform mat4 VPMat;
    auto params = ParameterMapNew();
    ParameterMapAppendMatrix4fv(params, Material->ShaderProgram->GetUniformLocation("MMat"), 1,
                                tc::Matrix4::IDENTITY.Data());
    ParameterMapAppendMatrix4fv(params, Material->ShaderProgram->GetUniformLocation("VPMat"), 1,
                                cameraParams.ViewProj.Data());
    cmdBuffer.SetParameterMap(params);

    // Bind vertex buffers from geometry
    Geometry->BindVertexBuffers(cmdBuffer);

    uint32_t first = 0, count = 2, instCount = LineSegmentCount * 4 - 2;
    // Generic BufferGeometry drawing code
    if (Geometry->GetIndexBuffer())
    {
        uint32_t indexSize = 4;
        if (Geometry->GetIndexType() == EBaseType::UInt16)
            indexSize = 2;
        if (instCount == 1)
        {
            cmdBuffer.DrawIndexedPrimitives(Geometry->GetTopology(), count, Geometry->GetIndexType(),
                                            Geometry->GetIndexBuffer(), indexSize * first);
        }
        else
        {
            throw "unimplemented";
        }
    }
    else
    {
        if (instCount == 1)
        {
            cmdBuffer.DrawPrimitives(Geometry->GetTopology(), first, count);
        }
        else
        {
            cmdBuffer.DrawPrimitives(Geometry->GetTopology(), first, count, instCount);
        }
    }
}

void CGridObject::MakeGeometry()
{
    float pos[6] = { 0.f, 0.f, 0.f, 1.f, 0.f, 0.f };
    auto posBuf = std::make_shared<CBuffer>(EBufferType::Vertex, sizeof(pos), pos);
    std::vector<float> srtData;
    float len = 2.0f * LineSpacing * (float)(LineSegmentCount - 1);
    for (int i = 1 - LineSegmentCount; i < LineSegmentCount; i++)
    {
        // Scale
        srtData.push_back(len);
        // Rotation in radians
        srtData.push_back(0.0f);
        // Origin of the line segment
        srtData.push_back(-len / 2.0f);
        srtData.push_back(0.0f);
        srtData.push_back(LineSpacing * static_cast<float>(i));
    }
    for (int i = 1 - LineSegmentCount; i < LineSegmentCount; i++)
    {
        srtData.push_back(len);
        srtData.push_back(3.141592654f / 2.0f);
        srtData.push_back(LineSpacing * static_cast<float>(i));
        srtData.push_back(0.0f);
        srtData.push_back(len / 2.0f);
    }
    auto srtBuf = std::make_shared<CBuffer>(EBufferType::Vertex, srtData.size() * sizeof(float), srtData.data());
    std::vector<CBufferDesc> bufferDescs = { { posBuf, 0, 12, 0 }, { srtBuf, 0, 20, 1 } };
    std::vector<CAttributeDesc> attributeDescs = { { EVertexAttribute::Index0, 0, EBaseType::Float, 3, 0 },
                                                   { EVertexAttribute::Index1, 1, EBaseType::Float, 2, 0 },
                                                   { EVertexAttribute::Index2, 1, EBaseType::Float, 3, 8 } };
    Geometry = std::make_shared<CBufferGeometry>(std::move(bufferDescs), std::move(attributeDescs),
                                                 EPrimitiveTopology::LineList);
}

void CGridObject::MakeMaterial()
{
    Material = std::make_shared<CMaterial>();
    Material->ShaderProgram = ParentCtx->GetShaderManager().GetShaderProgramByNames("LineInstanced", "LineColor");
    Material->polygonMode = EPolygonMode::Line;
    Material->lineWidth = 1.0f;
    Material->depthTestEnable = true;

    Material->bufferBindingDescs = Geometry->GetBufferBindings();
    Material->attributeDescs = Geometry->GetAttributes();
}
