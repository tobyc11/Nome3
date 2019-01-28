#include "Scene.h"
#include "Mesh.h"
#include <Render/Renderer.h>
#include <Render/Viewport.h>
#include <StringUtils.h>
#include <imgui.h>

namespace Nome::Scene
{

CScene::CScene()
{
    RootNode = new CSceneNode("root", true);
	CameraView = new Flow::TNumber<Matrix3x4>();
	CreateDefaultCamera();
}

void CScene::ConnectCameraTransform(Flow::TOutput<Matrix3x4>* output)
{
	if (output)
		output->Connect(CameraNode->Transform);
	else
		CameraView->Value.Connect(CameraNode->Transform);
}

void CScene::SetMainCameraViewport(CViewport* viewport)
{
	Viewport = viewport;
}

void CScene::CreateDefaultCamera()
{
	CameraNode = RootNode->CreateChildNode("default_camera");
	CameraView->SetNumber(Matrix3x4({ 0.0f, 0.0f, 10.0f }, Quaternion::IDENTITY, 1.0f));
	CameraView->Value.Connect(CameraNode->Transform);
	MainCamera = new CCamera(*CameraNode->GetTreeNodes().cbegin());
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

TAutoPtr<CSceneNode> CScene::CreateGroup(const std::string& name)
{
	if (Groups.find(name) != Groups.end())
		return {};

	auto* node = new CSceneNode(name);
	Groups[name] = node;
	return node;
}

TAutoPtr<CSceneNode> CScene::FindGroup(const std::string& name) const
{
	auto iter = Groups.find(name);
	if (iter != Groups.end())
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

	size_t charsToIgnore = 0;
	if (id[0] == '.')
		charsToIgnore = 1;
	//Descend down the scene tree starting from the root
	CSceneTreeNode* currNode = *RootNode->GetTreeNodes().begin();
	while (true)
	{
		CSceneTreeNode* nextNode = nullptr;
		size_t nextDot = id.find('.', charsToIgnore);
		if (nextDot != std::string::npos)
		{
			std::string nextSeg = id.substr(charsToIgnore, nextDot - charsToIgnore);
			nextNode = currNode->FindChild(nextSeg);
		}

		if (!nextNode)
		{
			if (auto* meshInstance = dynamic_cast<CMeshInstance*>(currNode->GetInstanceEntity()))
			{
				std::string idTurnedVertName = id;
				std::replace(idTurnedVertName.begin(), idTurnedVertName.end(), '.', '_');
				auto* point = meshInstance->CreateVertexSelector(id.substr(charsToIgnore), idTurnedVertName);
				if (point)
					return &point->Point;
				else
					return nullptr;
			}
			else
				return nullptr;
		}
		currNode = nextNode;
		charsToIgnore = nextDot + 1;
	}
    return nullptr;
}

std::pair<CSceneTreeNode*, std::string> CScene::WalkPath(const std::string& path) const
{
	auto pathComps = tc::FStringUtils::Split(path, ".");
	auto iter = pathComps.begin();
	if (iter->empty())
		++iter;
	CSceneTreeNode* currNode = *RootNode->GetTreeNodes().begin();
	for (; iter != pathComps.end(); ++iter)
	{
		CSceneTreeNode* childNode = currNode->FindChild(*iter);
		if (!childNode)
			break;
		currNode = childNode;
	}
	return { currNode, tc::FStringUtils::Combine(iter, pathComps.end(), ".") };
}

void DFSTreeNode(CSceneTreeNode* treeNode)
{
    if (ImGui::TreeNode(treeNode->GetOwner()->GetName().c_str()))
    {
		if (CEntity* instEnt = treeNode->GetInstanceEntity())
		{
			ImGui::Text("Instance Entity=%s", instEnt->GetName().c_str());
			bool validity = instEnt->IsEntityValid();
			ImGui::Checkbox("Validity", &validity);
			ImGui::Text("Update count %d", instEnt->GetUpdateCount());
		}
		if (CEntity* ent = treeNode->GetOwner()->GetEntity())
		{
			ImGui::Text("Entity=%s", ent->GetName().c_str());
			bool validity = ent->IsEntityValid();
			ImGui::Checkbox("Validity", &validity);
			ImGui::Text("Update count %d", ent->GetUpdateCount());
		}

        const auto& childNodes = treeNode->GetChildren();
        for (CSceneTreeNode* child : childNodes)
            DFSTreeNode(child);
        ImGui::TreePop();
    }
}

void CScene::ImGuiUpdate()
{
	BankAndSet.DrawImGui();

    ImGui::Begin("Scene Viewer");
	ImGui::ColorEdit4("Clear Color", const_cast<float*>(ClearColor.Data()));
    if (ImGui::CollapsingHeader("Entity"))
    {
        for (const auto& ent : EntityLibrary)
        {
            ImGui::Text("%s", ent.second->GetName().c_str());
        }
    }
    if (ImGui::CollapsingHeader(("Scene Tree")))
    {
        const auto& rootTreeNodes = RootNode->GetTreeNodes();
        assert(rootTreeNodes.size() == 1); //There is only one way to the root, thus only one tree node
        DFSTreeNode(*rootTreeNodes.begin());
    }
    ImGui::End();
}

void DFSTreeNodeUpdate(CSceneTreeNode* treeNode)
{
	treeNode->L2WTransform.Update();

	const auto& childNodes = treeNode->GetChildren();
	for (CSceneTreeNode* child : childNodes)
		DFSTreeNodeUpdate(child);

	if (auto* instEnt = treeNode->GetInstanceEntity())
	{
		//Update the instance entity
		instEnt->UpdateEntity();
	}
	else if (auto* ent = treeNode->GetOwner()->GetEntity())
	{
		//Otherwize, update the scene node entity
		ent->UpdateEntity();
	}
}

void CScene::Update()
{
	//Called every frame to make sure everything is up to date

	const auto& rootTreeNodes = RootNode->GetTreeNodes();
	assert(rootTreeNodes.size() == 1); //There is only one way to the root, thus only one tree node
	DFSTreeNodeUpdate(*rootTreeNodes.begin());

	if (Viewport)
		MainCamera->SetAspectRatio(Viewport->GetAspectRatio());
}

void DFSTreeNodeRender(CSceneTreeNode* treeNode)
{
	const auto& childNodes = treeNode->GetChildren();
	for (CSceneTreeNode* child : childNodes)
		DFSTreeNodeRender(child);

	if (auto* instEnt = treeNode->GetInstanceEntity())
	{
		instEnt->Draw(treeNode);
	}
	else if (auto* ent = treeNode->GetOwner()->GetEntity())
	{
		ent->Draw(treeNode);
	}
}

void CScene::Render()
{
	//We don't render without a viewport
	if (!Viewport)
		return;

	GRenderer->BeginView(MainCamera->GetViewMatrix(), MainCamera->GetProjMatrix(), Viewport, ClearColor);
	const auto& rootTreeNodes = RootNode->GetTreeNodes();
	assert(rootTreeNodes.size() == 1); //There is only one way to the root, thus only one tree node
	DFSTreeNodeRender(*rootTreeNodes.begin());
	GRenderer->EndView();
	GRenderer->Render();
}

}
