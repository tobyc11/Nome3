#include "InteractiveLight.h"
#include "Nome3DView.h"
#include "ResourceMgr.h"
#include <Scene/Light.h>

namespace Nome
{

CInteractiveLight::CInteractiveLight(Scene::CSceneTreeNode* node)
    : SceneTreeNode(node)
{
    UpdateLight();
}

void CInteractiveLight::UpdateTransform() {
    if (type == DirectionalLight) {
        auto *dLight = dynamic_cast<Qt3DRender::QDirectionalLight *>(Light);
        const auto &tf = SceneTreeNode->L2WTransform.GetValue(tc::Matrix3x4::IDENTITY);
        QMatrix4x4 qtf{tf.ToMatrix4().Data()};
        dLight->setWorldDirection(qtf * QVector3D(0, 1, 0));
    }
}

void CInteractiveLight::UpdateLight()
{
    auto* entity = SceneTreeNode->GetInstanceEntity();
    if (!entity)
    {
        entity = SceneTreeNode->GetOwner()->GetEntity();
    }
    if (entity)
    {
        auto LightInstance = dynamic_cast<Scene::CLight*>(entity)->GetLight();
        if (LightInstance.type == "NOME_DIRECTIONAL") {
            if (!Light)
                Light = new Qt3DRender::QDirectionalLight();
            Light->setColor(LightInstance.color);
            type = DirectionalLight;
        } else if (LightInstance.type == "NOME_AMBIENT") {
            Color = LightInstance.color;
            type = AmbientLight;
        } else if (LightInstance.type == "NOME_SPOT") {
            if (!Light)
                Light = new Qt3DRender::QSpotLight();
            Light->setColor(LightInstance.color);
            type = SpotLight;
        } else if (LightInstance.type == "NOME_POINT"){
            if (!Light)
                Light = new Qt3DRender::QPointLight();
            Light->setColor(LightInstance.color);
            type = PointLight;
        }
        else
        {
            std::cout << "The entity is not a Light instance, we don't know how to handle it. For "
                         "example, if you try to instanciate a face, it'll generate this "
                         "placeholder sphere."
                      << std::endl;
        }
    }
}
}