#pragma once
#include "BufferGeometry.h"
#include "CommandBuffer.h"
#include <Matrix4.h>
#include <memory>

class CGridObject;

struct CCameraParams
{
    tc::Matrix4 ProjMat, ViewMat;
    tc::Matrix4 ViewProj, InvViewProj;
    tc::Vector3 CameraPos;
};

class CDrawList : public CRenderContextChild
{
public:
    CDrawList();

    void Clear(float r, float g, float b, float a);
    void SetCamera(const tc::Matrix4& projMat, const tc::Matrix4& viewMat);
    void SetModelMat(const tc::Matrix4& modelMat);
    void SetPointLight(const tc::Vector3& position);
    void SetDirectionalLight();
    void DrawGeometry(CBufferGeometry& geometry, uint32_t vertexCount);
    void DrawWireframe(CBufferGeometry& geometry, uint32_t vertexCount);
    void Wait();

    // Additional features
    void DrawGradientBackground(const tc::Vector3& topColor, const tc::Vector3& bottomColor);
    void DrawGrid();

protected:
    void UpdateLightingEnv();

private:
    CCommandBuffer& GetCmdBuffer();

    std::shared_ptr<CCommandBuffer> CmdBuffer;
    CCameraParams cameraParams;
    tc::Matrix4 ModelMat;
    tc::Matrix3 NormalMat;
    std::shared_ptr<CMaterial> solidMaterial;
    std::shared_ptr<CMaterial> wireframeMaterial;
    std::vector<uint8_t> LightingEnvBlock;
    std::shared_ptr<CBuffer> LightingEnvUB;

    // Additional objects that we can draw
    std::shared_ptr<CGridObject> GridObject;
};
