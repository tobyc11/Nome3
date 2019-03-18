#pragma once

#include "DynamicScene.h"

#include <Buffer.h>
#include <DrawTemplate.h>
#include <UniformBuffer.h>
#include <Viewport.h>

namespace Nome::Render
{

using RHI::CBuffer;
using RHI::CDrawTemplate;
using tc::sp;

struct CBCameraParams
{
    alignas(RHI::TUniformBufferMemberTraits<tc::Matrix4>::Align)
        tc::Matrix4 ViewMat;
    alignas(RHI::TUniformBufferMemberTraits<tc::Matrix4>::Align)
        tc::Matrix4 ProjMat;
    alignas(RHI::TUniformBufferMemberTraits<tc::Vector3>::Align)
        tc::Vector3 CameraPos;

    static constexpr size_t RoundSize()
    {
        return (sizeof(CBCameraParams) + 15) / 16 * 16;
    }
};

class CSceneView
{
public:
    CSceneView(RHI::IViewport& vp, CDynamicScene& scene);

    void SetViewMat(const tc::Matrix4& value);
    void SetProjMat(const tc::Matrix4& value);
    void SetFrustum(const tc::Frustum& value);

    tc::Frustum GetFrustum() const;
    CDynamicScene& GetScene() const;
    RHI::IViewport& GetViewport() const;

    void BindToDrawTemplate(CDrawTemplate& drawTemplate) const;

private:
    CDynamicScene& Scene;
    RHI::IViewport& Viewport;

    tc::Frustum Frustum;
    CBCameraParams CameraParams;
    mutable bool bCameraParamsDirty = true;
    static sp<CBuffer> CameraParamsBuffer;
};

} // namespace Nome::Render
