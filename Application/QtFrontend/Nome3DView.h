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

     // Randy added on 10/14 for face selection
    [[nodiscard]] const std::vector<std::string>& GetSelectedFaces() const { return SelectedFaces; }

    // Randy added on 11/5 for edge selection
    [[nodiscard]] const std::vector<std::string>& GetSelectedEdgeVertices() const
    {
        return SelectedEdgeVertices;
    }

    void ClearSelectedVertices(); // Randy added on 9/27
    void ClearSelectedFaces(); // Randy added on 10/14 for deselecting faces
    void ClearSelectedEdges(); // Randy added on 11/5 for deselecting edges
    void TakeScene(const tc::TAutoPtr<Scene::CScene>& scene);
    void UnloadScene();
    void PostSceneUpdate();
    void PickVertexWorldRay(const tc::Ray& ray);
    void PickFaceWorldRay(const tc::Ray& ray); // Randy added on 10/10
    void PickEdgeWorldRay(const tc::Ray& ray); // Randy added on 10/29

    bool WireFrameMode = false; // Randy added on 10/16 for choose wireframe mode or default mode.
                                // Changed to false on 11/21

    void FreeVertexSelection();


    static Qt3DCore::QEntity* MakeGridEntity(Qt3DCore::QEntity* parent);

    bool PickVertexBool = false; // Randy added on 11/5
    bool PickFaceBool = false; // Randy added on 11/5
    bool PickEdgeBool = false; // Randy added on 11/5

protected:
    // Xinyu added on Oct 8 for rotation
    void mouseMoveEvent(QMouseEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void wheelEvent(QWheelEvent *ev) override;
    void keyPressEvent(QKeyEvent *ev) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QVector2D GetProjectionPoint(QVector2D originalPosition);
    static QVector3D GetCrystalPoint(QVector2D originalPoint);
    void rotateRay(tc::Ray& ray);
private:
    Qt3DCore::QEntity* Root;
    Qt3DCore::QEntity* Base;
    tc::TAutoPtr<Scene::CScene> Scene;
    std::unordered_set<CInteractiveMesh*> InteractiveMeshes;
    std::unordered_map<Scene::CEntity*, CDebugDraw*> EntityDrawData;
    std::vector<std::string> SelectedVertices;
    std::vector<std::string> SelectedFaces; // Randy added on 10/10
    std::vector<std::string> SelectedEdgeVertices; // Randy added on 11/5
    // std::vector<const & std::vector<std::string>> SelectedEdgeVertPositions; // There are no edge "names" right now TODO: Introduce Edge names and handles
    bool vertexSelectionEnabled;

    // Xinyu added on Oct 8 for rotation
    QMatrix4x4 projection;
    QVector2D firstPosition;
    QVector2D secondPosition;

    QQuaternion rotation;
    Qt3DRender::QCamera *cameraset;
    // Qt3DRender::QMaterial *material;
    bool mousePressEnabled;

    bool rotationEnabled;

    bool animationEnabled;
    float zPos;

    float objectX;
    float objectY;
    float objectZ;


    // For the animation
    Qt3DCore::QTransform *sphereTransform;
    Qt3DCore::QTransform *torusTransformX;
    Qt3DCore::QTransform *torusTransformY;
    Qt3DCore::QTransform *torusTransformZ;
    QQuaternion quaternionX;
    QQuaternion quaternionY;
    OrbitTransformController *controller;
    QPropertyAnimation *sphereRotateTransformAnimation;
    Qt3DCore::QEntity *torusX;
    Qt3DCore::QEntity *torusY;
    Qt3DCore::QEntity *torusZ;
    Qt3DExtras::QPhongAlphaMaterial *materialX;
    Qt3DExtras::QPhongAlphaMaterial *materialY;
    Qt3DExtras::QPhongAlphaMaterial *materialZ;

};

}