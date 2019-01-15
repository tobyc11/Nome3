#include "Scene.h"
#include <imgui.h>

namespace Nome::Scene
{

CScene::CScene()
{
    RootNode = new CSceneNode("root");
}

void CScene::CreateDefaultCamera()
{
	auto cameraNode = RootNode->CreateChildNode("default_camera");
	MainCamera = new CCamera(*cameraNode->GetTreeNodes().cbegin());
}

void CScene::AddEntity(TAutoPtr<CEntity> entity)
{
    EntityLibrary.insert(std::make_pair(entity->GetName(), entity));
}

TAutoPtr<CEntity> CScene::FindEntity(const std::string& name) const
{
    auto iter = EntityLibrary.find(name);
    if (iter != EntityLibrary.end())
        return iter->second;
    return nullptr;
}

Flow::TOutput<CVertexInfo*>* CScene::FindPointOutput(const std::string& id) const
{
    auto iter = EntityLibrary.find(id);
    if (iter != EntityLibrary.end())
    {
        TAutoPtr<CEntity> ent = iter->second;
        TAutoPtr<CPoint> point = dynamic_cast<CPoint*>(ent.Get());
        if (point)
        {
            return &point->Point;
        }
    }
    //TODO: find points within scene instances
    return nullptr;
}

void CScene::Update()
{
	//TODO: THE MOST IMPORTANT FUNCTION IN THIS MODULE
	//Called every frame to make sure everything is up to date
}

void CScene::Render()
{
}

void CScene::ImGuiUpdate()
{
	ImGui::Begin("Scene Viewer");
	if (ImGui::CollapsingHeader("Entity"))
	{
		for (const auto& ent : EntityLibrary)
		{
			ImGui::Text("%s", ent.second->GetName().c_str());
		}
	}
	ImGui::End();
}

}
