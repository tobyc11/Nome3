#include "EditorViewportClient.h"
#include <Scene/InteractivePoint.h>
#include <Scene/Mesh.h>
#include <Render/Renderer.h>
#include <Render/GraphicsDevice.h>
#include <sstream>
#include <imgui.h>

namespace Nome
{

CEditorViewportClient::CEditorViewportClient(tc::TAutoPtr<Scene::CScene> scene, sp<CSourceManager> sm, CSourceFile* sf)
    : Scene(scene), SourceManager(sm), SourceFile(sf)
{
    //Create a camera controller and connect it to the scene
    OrbitCameraController = new Scene::COrbitCameraController();
    Scene->ConnectCameraTransform(&OrbitCameraController->Transform);

    memset(MeshName, 0, sizeof(MeshName));
}

void CEditorViewportClient::Draw(CViewport* vp)
{
    //TODO: logical?
    Scene->SetMainCameraViewport(vp);

    {
        ImGui::Begin("Nome Debug");
        ImGui::Text("Display Adapter:");
        ImGui::Text("%s", GRenderer->GetGD()->GetDescription().c_str());
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }
    {
        //TODO: messy
        ImGui::Begin("Interactivity");
        ImGui::Text("This is still work in progress...");
        if (ImGui::Button("Abandon Face"))
        {
            PendingFace.clear();
        }
        if (ImGui::Button("End Face"))
        {
            PendingMesh.push_back(PendingFace);
            PendingFace.clear();
        }
        ImGui::Text("Currently has %d faces", PendingMesh.size());
        ImGui::InputText("Mesh Name", MeshName, sizeof(MeshName));
        if (ImGui::Button("End Mesh"))
        {
            std::stringstream ss;
            ss << "mesh " << MeshName << std::endl;
            int counter = 0;
            for (const auto& faceVec : PendingMesh)
            {
                ss << "    face f" << std::to_string(counter++) << " (";
                for (const auto& vertName : faceVec)
                {
                    ss << " " << vertName;
                }
                ss << ") endface" << std::endl;
            }
            ss << "endmesh" << std::endl;
            ss << "instance i_" << MeshName << " " << MeshName << " endinstance" << std::endl;

            SourceManager->WriteLine(SourceFile, ss.str());
            PendingMesh.clear();
        }
        ImGui::End();
    }
    Scene->ImGuiUpdate();

    Scene->Update();

    // Rendering
    Scene->Render();
    GRenderer->Render();
}

bool CEditorViewportClient::OnMousePress(CViewport* vp, uint32_t buttons, int x, int y)
{
    if (buttons & 0b10)
    {
        CInteractivePoint* pointHit = Scene->GetPickingMgr()->Pick(x, y);
        //TODO: DANGER static_cast<Scene::CMeshInstancePoint*>
        if (auto* meshPoint = static_cast<Scene::CMeshInstancePoint*>(pointHit))
        {
            printf("Hit: %s\n", meshPoint->GetPointPath().c_str());
            PendingFace.push_back(meshPoint->GetPointPath());
        }
    }
    if (buttons & 0b100)
    {
        OrbitCameraController->Activate();
        LastX = x;
        LastY = y;
        return true;
    }
    return false;
}

bool CEditorViewportClient::OnMouseRelease(CViewport* vp, uint32_t buttons, int x, int y)
{
    if (buttons & 0b100)
    {
        OrbitCameraController->Inactivate();
        return true;
    }
    return false;
}

bool CEditorViewportClient::OnMouseMove(CViewport* vp, int x, int y)
{
    OrbitCameraController->MouseMoved(x - LastX, y - LastY);
    LastX = x;
    LastY = y;
    return true;
}

bool CEditorViewportClient::OnMouseWheel(CViewport* vp, int degrees)
{
    OrbitCameraController->WheelMoved(degrees);
    return true;
}

}
