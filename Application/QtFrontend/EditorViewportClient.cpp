#include "EditorViewportClient.h"
#include <Render/Renderer.h>
#include <Render/GraphicsDevice.h>
#include <imgui.h>

namespace Nome
{

CEditorViewportClient::CEditorViewportClient(tc::TAutoPtr<Scene::CScene> scene) : Scene(scene)
{
    //Create a camera controller and connect it to the scene
    OrbitCameraController = new Scene::COrbitCameraController();
    Scene->ConnectCameraTransform(&OrbitCameraController->Transform);
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
    Scene->ImGuiUpdate();

    Scene->Update();

    // Rendering
    Scene->Render();
    GRenderer->Render();
}

bool CEditorViewportClient::OnMousePress(CViewport* vp, uint32_t buttons, int x, int y)
{
    OrbitCameraController->Activate();
    LastX = x;
    LastY = y;
    return true;
}

bool CEditorViewportClient::OnMouseRelease(CViewport* vp, uint32_t buttons, int x, int y)
{
    OrbitCameraController->Inactivate();
    return true;
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
