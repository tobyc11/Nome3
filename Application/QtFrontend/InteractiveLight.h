#pragma once
#include <Scene/RendererInterface.h>
#include <Scene/SceneGraph.h>

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DRender/QMaterial>

namespace Nome
{

class CInteractiveLight : public Qt3DCore::QEntity
{
public:
    explicit CInteractiveLight(Scene::CSceneTreeNode* node);

    [[nodiscard]] Scene::CSceneTreeNode* GetSceneTreeNode() const { return SceneTreeNode; }

    void UpdateTransform();
    void UpdateLight();

    void InitInteractions();


private:
    Scene::CSceneTreeNode* SceneTreeNode = nullptr;

    Qt3DCore::QTransform* Transform = nullptr;

    Qt3DRender::QMaterial* Material = nullptr;
    Qt3DRender::QMaterial* LineMaterial = nullptr;

    Qt3DCore::QEntity* PointEntity;
    Qt3DRender::QMaterial* PointMaterial;
};

}
