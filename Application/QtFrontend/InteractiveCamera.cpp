#include "InteractiveCamera.h"
#include "Nome3DView.h"
#include "ResourceMgr.h"
#include <Scene/Camera.h>

namespace Nome
{

CInteractiveCamera::CInteractiveCamera(Scene::CSceneTreeNode* node)
    : SceneTreeNode(node)
{
    UpdateCamera();
}

void CInteractiveCamera::UpdateTransform() {
    const auto &tf = SceneTreeNode->L2WTransform.GetValue(tc::Matrix3x4::IDENTITY);
    QMatrix4x4 qtf{tf.ToMatrix4().Data()};

    Camera->setUpVector(QVector3D(0, 0, 1));
    Camera->setViewCenter(QVector3D(0, 0, 0));
    Camera->setProjectionMatrix(qtf);

}

void CInteractiveCamera::UpdateCamera()
{
    auto* entity = SceneTreeNode->GetInstanceEntity();
    if (!entity)
    {
        entity = SceneTreeNode->GetOwner()->GetEntity();
    }
    if (entity)
    {
        auto CameraInstance = dynamic_cast<Scene::CCamera*>(entity)->GetCamera();
        name = entity->GetNameWithoutPrefix();

        if (CameraInstance.type == "NOME_PERSPECTIVE") {
            if (!Camera)
                Camera = new Qt3DRender::QCamera();
            Camera->lens()->setOrthographicProjection(CameraInstance.para[0],
                                                      CameraInstance.para[1],
                                                      CameraInstance.para[2],
                                                      CameraInstance.para[3],
                                                      CameraInstance.para[4],
                                                      CameraInstance.para[5]);

            type = Perspective;
        } else if (CameraInstance.type == "NOME_FRUSTUM") {
            Camera->lens()->setFrustumProjection(CameraInstance.para[0],
                                                 CameraInstance.para[1],
                                                 CameraInstance.para[2],
                                                 CameraInstance.para[3],
                                                 CameraInstance.para[4],
                                                 CameraInstance.para[5]);

            type = FRUSTUM;
        }
        else
        {
            std::cout << "The entity is not a Camera instance, we don't know how to handle it."
                      << std::endl;
        }
    }
}
}