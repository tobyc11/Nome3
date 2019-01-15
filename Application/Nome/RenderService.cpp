#include "RenderService.h"
#include "Scene/Entity.h"

#include <glad/glad.h>

#include "Shader.h"

//Optimus enablement
extern "C"
{
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

namespace Nome
{

using tc::FGLSLProgram;

static FGLSLProgram* MeshShader;

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
    FGLSLProgram::SetShaderHeader("");
    MeshShader = FGLSLProgram::CreateFromFiles("mesh.vert", "mesh.frag");
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
    const auto& frustum = camera->GetFrustum();

    //Bind a simple pipeline right now
    MeshShader->Enable();
    MeshShader->SetUniformMatrix4fv("View", camera->GetViewMatrix().Data(), 1, true);
    MeshShader->SetUniformMatrix4fv("Proj", camera->GetProjMatrix().Data(), 1, true);

    /*
    const auto& projInv = camera->GetProjMatrix().Inverse();
    float ends[] = {-1.0f, 1.0f};
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
            for (int k = 0; k < 2; k++)
            {
                Vector3 point(ends[i], ends[j], ends[k]);
                Vector3 unProj = projInv * point;
                std::cout << unProj.ToString() << std::endl;
            }
    */

    //Rasterizer
    glDisable(GL_CULL_FACE);
    //glFrontFace(GL_CCW);
    //glCullFace(GL_BACK);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    //FB
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_BLEND);

    //auto sceneRoot = Scene->GetRootNode();
    //assert(sceneRoot->CountTreeNodes() == 1);
    //auto sceneTreeRoot = *sceneRoot->GetTreeNodes().begin();
    //sceneTreeRoot->ForEach([](CSceneTreeNode* node){
    //    //TODO: efficient frustum testing

    //    auto* graphNode = node->GetOwner();
    //    if (const auto& entity = graphNode->GetEntity())
    //    {
    //        MeshShader->SetUniformMatrix4fv("Model", node->GetL2W().ToMatrix4().Data(), 1, true);
    //        entity->Draw();
    //    }
    //});
}

int CRenderService::Cleanup()
{
    delete MeshShader;
    MeshShader = nullptr;
    return 0;
}

}
