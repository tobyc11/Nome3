#pragma once
#include "DebugDraw.h"
#include "InteractiveMesh.h"
#include <Scene/Scene.h>
#include <Ray.h>

#include <Qt3DExtras>

#include <unordered_map>
#include <unordered_set>

namespace Nome
{

class CNome3DView : public Qt3DExtras::Qt3DWindow
{
public:
    CNome3DView();
    ~CNome3DView() override;

    void TakeScene(const tc::TAutoPtr<Scene::CScene>& scene);
    void UnloadScene();
    void PostSceneUpdate();

    void PickVertexWorldRay(const tc::Ray& ray);

    static Qt3DCore::QEntity* MakeGridEntity(Qt3DCore::QEntity* parent);

private:
    Qt3DCore::QEntity* Root;
    tc::TAutoPtr<Scene::CScene> Scene;
    std::unordered_set<CInteractiveMesh*> InteractiveMeshes;
    std::unordered_map<Scene::CEntity*, CDebugDraw*> EntityDrawData;
};

}
