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
        if (CameraInstance.type == "NOME_PERSPECTIVE") {
            if (!Camera)
                Camera = new Qt3DRender::QDirectionalCamera();
            Camera->setColor(CameraInstance.color);
            type = DirectionalCamera;
        } else if (CameraInstance.type == "NOME_AMBIENT") {
            Color = CameraInstance.color;
            type = AmbientCamera;
        }
        else
        {
            std::cout << "The entity is not a Camera instance, we don't know how to handle it. For "
                         "example, if you try to instanciate a face, it'll generate this "
                         "placeholder sphere."
                      << std::endl;
        }
    }
}
}