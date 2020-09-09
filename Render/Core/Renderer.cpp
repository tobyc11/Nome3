#include "Renderer.h"
#include "DirectLighting.h"
#include "ShaderCombiner.h"

#include <RenderGraph.h>

namespace Nome::Render
{

void CRenderer::Render(const CSceneView& sceneView)
{
    auto& drawPassGen = sceneView.GetViewport().GetRenderGraph()->GetRenderPass("drawPass"_hash);
    auto& drawPass = static_cast<RHI::CDrawPass&>(drawPassGen);

    auto frustum = sceneView.GetFrustum();
    auto& scene = sceneView.GetScene();
    auto objs = scene.Cull(frustum);

    CDirectLightingModel lightingModel;

    drawPass.BeginRecording();
    for (CSceneObject* obj : objs)
    {
        auto iter = scene.ObjectPipelines.find(obj);
        if (iter == scene.ObjectPipelines.end())
        {
            CShaderCombiner sc;
            RHI::CDrawTemplate temp;
            sceneView.BindToDrawTemplate(temp);
            obj->BindSceneObjectParamsBuffer(temp);
            obj->GetShape()->ChooseShaders(sc);
            obj->GetShape()->BindPipelineArgs(temp);
            obj->GetMaterial()->ChooseShaders(sc);
            obj->GetMaterial()->BindPipelineArgs(temp);
            lightingModel.ChooseShaders(sc);
            lightingModel.BindToDrawTemplate(temp);
            sc.BindToDrawTemplate(temp);
            RHI::CPipelineStates pipeline = scene.PipelineCache->CreatePipelineStates(temp);
            scene.ObjectDrawDescs[obj] = temp;
            scene.ObjectPipelines[obj] = pipeline;
        }
        //The pipeline is now cached
        //Update constants and draw
        RHI::CDrawTemplate draw;
        RHI::CPipelineStates states = scene.ObjectPipelines[obj];
        sceneView.BindToDrawTemplate(draw);
        obj->BindSceneObjectParamsBuffer(draw);
        obj->GetShape()->BindPipelineArgs(draw);
        obj->GetMaterial()->BindPipelineArgs(draw);
        lightingModel.BindToDrawTemplate(draw);
        drawPass.Record(states, draw);
    }
    drawPass.FinishRecording();
}

} // namespace Nome::Render
