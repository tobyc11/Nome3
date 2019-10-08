#pragma once
#include "InteractiveMesh.h"
#include <Scene/Scene.h>

#include <Qt3DExtras>

#include <unordered_set>

namespace Nome
{

class CNome3DView : public Qt3DExtras::Qt3DWindow
{
public:
    CNome3DView();
    ~CNome3DView() override;

    void TakeScene(const tc::TAutoPtr<Scene::CScene>& Scene);
    void UnloadScene();
    void PostSceneUpdate(const tc::TAutoPtr<Scene::CScene>& Scene);

    static Qt3DCore::QEntity* MakeGridEntity(Qt3DCore::QEntity* parent);

private:
    Qt3DCore::QEntity* Root;
    std::unordered_set<CInteractiveMesh*> InteractiveMeshes;
};

}
