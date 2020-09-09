#include "SceneView.h"

#include <Device.h>
#include <RHIInstance.h>
#include <RenderGraph.h>

namespace Nome::Render
{

sp<CBuffer> CSceneView::CameraParamsBuffer;

CSceneView::CSceneView(RHI::IViewport& vp, CDynamicScene& scene)
    : Viewport(vp)
    , Scene(scene)
{
    //Setup render graph
    RHI::CRenderGraph& renderGraph = *Viewport.GetRenderGraph();

    RHI::CNodeId depthStencil = renderGraph.DeclareRenderTarget();

    sp<RHI::CDrawPass> drawPass = new RHI::CDrawPass(renderGraph);
    drawPass->SetColorAttachment(0, "swapchain"_hash);
    drawPass->SetDepthStencilAttachment(depthStencil);
    auto& clearPass = renderGraph.GetRenderPass("clearPass"_hash);
    clearPass.SetDepthStencilAttachment(depthStencil);
    drawPass->AddDependency(clearPass);
    auto& imguiPass = renderGraph.GetRenderPass("imguiPass"_hash); //TODO: this is so dependent on nome view
    imguiPass.AddDependency(*drawPass);
    renderGraph.AddRenderPass("drawPass"_hash, *drawPass);

    CameraParamsBuffer = RHI::CInstance::Get().GetCurrDevice()->CreateBuffer(
        (uint32_t)CBCameraParams::RoundSize(), RHI::EBufferUsageFlags::ConstantBuffer);
}

void CSceneView::SetViewMat(const tc::Matrix4& value)
{
    bCameraParamsDirty = true;
    CameraParams.ViewMat = value;
    CameraParams.CameraPos = -value.Translation();
}

void CSceneView::SetProjMat(const tc::Matrix4& value)
{
    bCameraParamsDirty = true;
    CameraParams.ProjMat = value;
}

void CSceneView::SetFrustum(const tc::Frustum& value) { Frustum = value; }

tc::Frustum CSceneView::GetFrustum() const { return Frustum; }

CDynamicScene& CSceneView::GetScene() const { return Scene; }

RHI::IViewport& CSceneView::GetViewport() const { return Viewport; }

void CSceneView::BindToDrawTemplate(CDrawTemplate& drawTemplate) const
{
    if (bCameraParamsDirty)
    {
        //Relies on the fact that this draw is the first one in the pass
        //  **might break
        RHI::CBufferUpdateRequest req;
        req.Buffer = CameraParamsBuffer;
        req.Size = (uint32_t)CBCameraParams::RoundSize();
        req.Data = &CameraParams;
        drawTemplate.GetBufferUpdateReqs().push_back(req);
        bCameraParamsDirty = false;
    }
    drawTemplate.GetPipelineArguments().Add("CameraParams"_hash, CameraParamsBuffer);
}

} // namespace Nome::Render
