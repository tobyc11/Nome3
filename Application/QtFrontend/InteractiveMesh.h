#pragma once
#include <Scene/SceneGraph.h>
#include <Scene/RendererInterface.h>

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DRender/QMaterial>

namespace Nome
{

class CInteractiveMesh : public Qt3DCore::QEntity, public IMeshRenderer
{
public:
    explicit CInteractiveMesh(Scene::CSceneTreeNode* node);

    void UpdateTransform();
    void UpdateGeometry();
    void UpdateMaterial();
    void InitInteractions();

    int GetRenderFlags() override;
    void NotifyGeometryChange() override;

private:
    Scene::CSceneTreeNode* SceneTreeNode = nullptr;

    Qt3DCore::QTransform* Transform = nullptr;
    Qt3DRender::QMaterial* Material = nullptr;
};

}
