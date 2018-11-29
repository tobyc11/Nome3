#include "RenderService.h"
#include "Scene/Entity.h"

#include <glad/glad.h>

namespace Nome
{

Scene::CScene* CRenderService::GetScene() const
{
    return Scene;
}

void CRenderService::SetScene(Scene::CScene* value)
{
    Scene = value;
}

int CRenderService::Setup()
{
    return 0;
}

void CRenderService::Render()
{
    using namespace Scene;

    if (!Scene)
        return;

    auto camera = Scene->GetMainCamera();
    if (!camera)
        return;
    auto cameraTreeNode = camera->GetPrincipleTreeNode();
    if (!cameraTreeNode)
        return;
    const auto& frustum = camera->GetFrustum();

    auto sceneRoot = Scene->GetRootNode();
    assert(sceneRoot->CountTreeNodes() == 1);
    auto sceneTreeRoot = *sceneRoot->GetTreeNodes().begin();
    sceneTreeRoot->ForEach([](CSceneTreeNode* node){
        //TODO: efficient frustum testing

        auto* graphNode = node->GetOwner();
        if (const auto& entity = graphNode->GetEntity())
        {
            entity->Draw();
        }
    });

    //Example pipeline setup
    //VS

    //Rasterizer
//    glEnable(GL_CULL_FACE);
//    glFrontFace(GL_CCW);
//    glCullFace(GL_BACK);

    //FS

    //FB
/*    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_BLEND);*/
}

}
