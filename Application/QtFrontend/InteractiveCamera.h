#pragma once
#include <Scene/RendererInterface.h>
#include <Scene/SceneGraph.h>

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <QCamera>

enum CameraType { Perspective, FRUSTUM };


namespace Nome
{

class CInteractiveCamera : public Qt3DCore::QEntity
{
public:
    explicit CInteractiveCamera(Scene::CSceneTreeNode* node);

    [[nodiscard]] Scene::CSceneTreeNode* GetSceneTreeNode() const { return SceneTreeNode; }

    void UpdateTransform();
    void UpdateCamera();

public:
    Qt3DRender::QCamera* Camera = nullptr;
    CameraType type;
    std::string name;


private:
    Scene::CSceneTreeNode* SceneTreeNode = nullptr;
};

}
