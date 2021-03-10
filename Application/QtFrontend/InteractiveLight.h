#pragma once
#include <Scene/RendererInterface.h>
#include <Scene/SceneGraph.h>

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DRender/QMaterial>
#include <QDirectionalLight>

enum LightType { SpotLight, AmbientLight, DirectionalLight, PointLight };

namespace Nome
{

class CInteractiveLight : public Qt3DCore::QEntity
{
public:
    explicit CInteractiveLight(Scene::CSceneTreeNode* node);

    [[nodiscard]] Scene::CSceneTreeNode* GetSceneTreeNode() const { return SceneTreeNode; }

    void UpdateTransform();
    void UpdateLight();

public:
    Qt3DRender::QAbstractLight* Light = nullptr;
    LightType type;
    QColor Color;

private:
    Scene::CSceneTreeNode* SceneTreeNode = nullptr;
};

}
