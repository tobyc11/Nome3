#pragma once
#include "Entity.h"
#include "SceneGraph.h"
#include "Camera.h"
#include "BankAndSet.h"
#include "Point.h"
#include <Color.h>
#include <utility>

namespace Nome
{
class CViewport;
}

namespace Nome::Scene
{

class CScene : public tc::FRefCounted
{
public:
    CScene();

    CBankAndSet& GetBankAndSet() { return BankAndSet; }

    TAutoPtr<CSceneNode> GetRootNode() const { return RootNode; }

    TAutoPtr<CCamera> GetMainCamera() const { return MainCamera; }
	void ConnectCameraTransform(Flow::TOutput<Matrix3x4>* output);
	void SetMainCameraViewport(CViewport* viewport);

    void CreateDefaultCamera();

    void AddEntity(TAutoPtr<CEntity> entity);
	//TODO:
    //void RemoveEntity(const std::string& name);
    //void RenameEntity(const std::string& oldName, const std::string& newName);
    TAutoPtr<CEntity> FindEntity(const std::string& name) const;

	TAutoPtr<CSceneNode> CreateGroup(const std::string& name);
	TAutoPtr<CSceneNode> FindGroup(const std::string& name) const;

    Flow::TOutput<CVertexInfo*>* FindPointOutput(const std::string& id) const;

	///Walks the scene tree along a path, return the last matching node and the rest of path
	std::pair<CSceneTreeNode*, std::string> WalkPath(const std::string& path) const;

    void ImGuiUpdate();

	void Update();

	void Render();

private:
	CBankAndSet BankAndSet;

    TAutoPtr<CSceneNode> RootNode;
	TAutoPtr<CSceneNode> CameraNode;
    TAutoPtr<CCamera> MainCamera;
	CViewport* Viewport = nullptr;
	tc::Color ClearColor;
	TAutoPtr<Flow::TNumber<Matrix3x4>> CameraView;

    std::map<std::string, TAutoPtr<CEntity>> EntityLibrary;
	std::map<std::string, TAutoPtr<CSceneNode>> Groups;
};

}
