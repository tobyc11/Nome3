#pragma once
#include "DebugDraw.h"
#include "InteractiveMesh.h"
#include "OrbitTransformController.h"
#include <Ray.h>
#include <Scene/Scene.h>

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

    [[nodiscard]] const std::vector<std::string>& GetSelectedVertices() const
    {
        return SelectedVertices;
    }

    void ClearSelectedVertices(); // Randy added on 9/27
    void TakeScene(const tc::TAutoPtr<Scene::CScene>& scene);
    void UnloadScene();
    void PostSceneUpdate();

    void PickVertexWorldRay(const tc::Ray& ray);

    static Qt3DCore::QEntity* MakeGridEntity(Qt3DCore::QEntity* parent);

protected:
    // Xinyu added on Oct 8 for rotation
    void mouseMoveEvent(QMouseEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void wheelEvent(QWheelEvent *ev) override;
    void keyPressEvent(QKeyEvent *ev) override;

private:
    Qt3DCore::QEntity* Root;
    tc::TAutoPtr<Scene::CScene> Scene;
    std::unordered_set<CInteractiveMesh*> InteractiveMeshes;
    std::unordered_map<Scene::CEntity*, CDebugDraw*> EntityDrawData;
    std::vector<std::string> SelectedVertices;

    // Xinyu added on Oct 8 for rotation
    QMatrix4x4 projection;
    QVector2D mousePressPosition;
    QVector3D rotationAxis;
    qreal angularSpeed;
    QQuaternion rotation;
    Qt3DRender::QCamera *cameraset;
    Qt3DExtras::QOrbitCameraController *camController;
    //Qt3DRender::QMaterial *material;
    bool mousePressEnabled;
    bool animationEnabled;
    float zPos;

    // For the animation
    Qt3DCore::QTransform *sphereTransform;
    OrbitTransformController *controller;
    QPropertyAnimation *sphereRotateTransformAnimation;

};

}
