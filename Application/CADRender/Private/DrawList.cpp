#include "DrawList.h"
#include "CGLThread.h"
#include "GridObject.h"
#include "ShaderManager.h"
#include <glad/glad.h>

CDrawList::CDrawList()
{
    // Note: this has to agree with the shader SolidMeshLit
    LightingEnvBlock.resize(32);
}

void CDrawList::Clear(float r, float g, float b, float a)
{
    ParentCtx->GetDevice().QueueTask([=]() {
        glClearColor(r, g, b, a);
        glClearDepth(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    });
}

void CDrawList::SetCamera(const tc::Matrix4& projMat, const tc::Matrix4& viewMat)
{
    cameraParams.ProjMat = projMat;
    cameraParams.ViewMat = viewMat;
    cameraParams.ViewProj = cameraParams.ProjMat * cameraParams.ViewMat;
    cameraParams.InvViewProj = cameraParams.ViewProj.Inverse();
    cameraParams.CameraPos = cameraParams.InvViewProj.Translation();
    // LightingEnv
    auto* cameraPos = (tc::Vector3*)(LightingEnvBlock.data() + 0);
    *cameraPos = cameraParams.CameraPos;
}

void CDrawList::SetModelMat(const tc::Matrix4& modelMat)
{
    ModelMat = modelMat;
    NormalMat = ModelMat.ToMatrix3().Inverse().Transpose();
}

void CDrawList::SetPointLight(const tc::Vector3& position)
{
    auto* lightPos = (tc::Vector3*)(LightingEnvBlock.data() + 16);
    *lightPos = position;
    UpdateLightingEnv();
}

void CDrawList::SetDirectionalLight() { throw "not implemented"; }

void CDrawList::DrawGeometry(CBufferGeometry& geometry, uint32_t vertexCount)
{
    if (!solidMaterial)
    {
        solidMaterial = std::make_shared<CMaterial>();
        auto program = ParentCtx->GetShaderManager().GetShaderProgramByNames("SolidMesh", "SolidMeshLit");
        solidMaterial->ShaderProgram = program;
        solidMaterial->bufferBindingDescs = geometry.GetBufferBindings();
        solidMaterial->attributeDescs = geometry.GetAttributes();
        // Verify that the attributes provided by the geometry is what the shader expects
        assert(solidMaterial->attributeDescs.size() == 2);
        assert(solidMaterial->attributeDescs[0].Attribute == EVertexAttribute::Position);
        assert(solidMaterial->attributeDescs[1].Attribute == EVertexAttribute::Normal);
        solidMaterial->depthTestEnable = true;
    }
    GetCmdBuffer().BindMaterial(solidMaterial);
    auto params = ParameterMapNew();
    ParameterMapAppendMatrix4fv(params, solidMaterial->ShaderProgram->GetUniformLocation("MMat"), 1, ModelMat.Data());
    ParameterMapAppendMatrix4fv(params, solidMaterial->ShaderProgram->GetUniformLocation("VPMat"), 1,
                                cameraParams.ViewProj.Data());
    ParameterMapAppendMatrix3fv(params, solidMaterial->ShaderProgram->GetUniformLocation("NormalMat"), 1,
                                NormalMat.Data());
    ParameterMapAppendUniformBuffer(params, solidMaterial->ShaderProgram->GetUniformLocation("LightingEnv"),
                                    LightingEnvUB);
    GetCmdBuffer().SetParameterMap(params);
    geometry.BindVertexBuffers(GetCmdBuffer());
    GetCmdBuffer().DrawPrimitives(geometry.GetTopology(), 0, vertexCount);
}

void CDrawList::DrawWireframe(CBufferGeometry& geometry, uint32_t vertexCount)
{
    if (!wireframeMaterial)
    {
        wireframeMaterial = std::make_shared<CMaterial>();
        auto program = ParentCtx->GetShaderManager().GetShaderProgramByNames("SolidMesh", "SolidMeshWire");
        wireframeMaterial->ShaderProgram = program;
        wireframeMaterial->bufferBindingDescs = geometry.GetBufferBindings();
        wireframeMaterial->attributeDescs = geometry.GetAttributes();
        // Verify that the attributes provided by the geometry is what the shader expects
        assert(wireframeMaterial->attributeDescs.size() == 2);
        assert(wireframeMaterial->attributeDescs[0].Attribute == EVertexAttribute::Position);
        assert(wireframeMaterial->attributeDescs[1].Attribute == EVertexAttribute::Normal);
        wireframeMaterial->polygonMode = EPolygonMode::Line;
        wireframeMaterial->depthTestEnable = true;
        wireframeMaterial->depthCompareOp = ECompareOp::LEqual;
    }
    GetCmdBuffer().BindMaterial(wireframeMaterial);
    auto params = ParameterMapNew();
    ParameterMapAppendMatrix4fv(params, wireframeMaterial->ShaderProgram->GetUniformLocation("MMat"), 1,
                                ModelMat.Data());
    ParameterMapAppendMatrix4fv(params, wireframeMaterial->ShaderProgram->GetUniformLocation("VPMat"), 1,
                                cameraParams.ViewProj.Data());
    ParameterMapAppendMatrix3fv(params, wireframeMaterial->ShaderProgram->GetUniformLocation("NormalMat"), 1,
                                NormalMat.Data());
    ParameterMapAppend4f(params, wireframeMaterial->ShaderProgram->GetUniformLocation("BaseColor"),
                         tc::Vector4(0.3f, 0.8f, 1.0f, 1.0f).Data());
    GetCmdBuffer().SetParameterMap(params);
    geometry.BindVertexBuffers(GetCmdBuffer());
    GetCmdBuffer().DrawPrimitives(geometry.GetTopology(), 0, vertexCount);
}

void CDrawList::Wait() { GetCmdBuffer().Submit(); }

void CDrawList::DrawGradientBackground(const tc::Vector3& topColor, const tc::Vector3& bottomColor)
{
    // Note, this calculation assumes depth in NDC goes from negative to positive 1
    auto upClose = cameraParams.InvViewProj * tc::Vector3(0.0f, 1.0f, 0.5f);
    auto upFar = cameraParams.InvViewProj * tc::Vector3(0.0f, 1.0f, 0.7f);
    auto up = upFar - upClose;
    up.Normalize();
    auto downClose = cameraParams.InvViewProj * tc::Vector3(0.0f, -1.0f, 0.5f);
    auto downFar = cameraParams.InvViewProj * tc::Vector3(0.0f, -1.0f, 0.7f);
    auto down = downFar - downClose;
    down.Normalize();

    // Now, interpolate the colors based on y axis of each direction
    auto t = (up.y + 1.0f) / 2.0f;
    auto screenTopColor = topColor * t + bottomColor * (1.0f - t);
    t = (down.y + 1.0f) / 2.0f;
    auto screenBottomColor = topColor * t + bottomColor * (1.0f - t);

    // Fire up the corresponding GL calls
    auto program = ParentCtx->GetShaderManager().GetShaderProgramByNames("FullscreenQuad", "FullscreenGradient");
    auto params = ParameterMapNew();
    ParameterMapAppend3f(params, program->GetUniformLocation("TopColor"), screenTopColor.Data());
    ParameterMapAppend3f(params, program->GetUniformLocation("BottomColor"), screenBottomColor.Data());
    GetCmdBuffer().BindProgram(std::move(program));
    GetCmdBuffer().SetParameterMap(std::move(params));
    GetCmdBuffer().DrawPrimitives(EPrimitiveTopology::TriangleFan, 0, 4);
}

CCommandBuffer& CDrawList::GetCmdBuffer()
{
    if (!CmdBuffer)
    {
        CmdBuffer = std::make_shared<CCommandBuffer>();
        CmdBuffer->AssertEqCtx(this->ParentCtx);
    }
    return *CmdBuffer;
}

void CDrawList::DrawGrid()
{
    if (!GridObject)
    {
        GridObject = std::make_shared<CGridObject>();
    }
    GridObject->Draw(GetCmdBuffer(), cameraParams);
}

void CDrawList::UpdateLightingEnv()
{
    // TODO: update lazily?
    LightingEnvUB = std::make_unique<CBuffer>(EBufferType::Uniform, LightingEnvBlock.size(), LightingEnvBlock.data());
}
