#pragma once
#include "Entity.h"
#include "SceneGraph.h"
#include "Camera.h"
#include "BankAndSet.h"
#include "Point.h"

namespace Nome::Scene
{

class CScene : public tc::FRefCounted
{
public:
    CScene();

    CBankAndSet& GetBankAndSet() { return BankAndSet; }

    TAutoPtr<CSceneNode> GetRootNode() const { return RootNode; }

    TAutoPtr<CCamera> GetMainCamera() const { return MainCamera; }

    void CreateDefaultCamera();

    void AddEntity(TAutoPtr<CEntity> entity);
	//TODO:
    //void RemoveEntity(const std::string& name);
    //void RenameEntity(const std::string& oldName, const std::string& newName);
    TAutoPtr<CEntity> FindEntity(const std::string& name) const;

    Flow::TOutput<CVertexInfo*>* FindPointOutput(const std::string& id) const;

	void Update();

	void Render();

	void ImGuiUpdate();

private:
	CBankAndSet BankAndSet;

    TAutoPtr<CSceneNode> RootNode;
    TAutoPtr<CCamera> MainCamera;

    std::map<std::string, TAutoPtr<CEntity>> EntityLibrary;
};

}
