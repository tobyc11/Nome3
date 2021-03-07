#include "InteractiveLight.h"
#include "Nome3DView.h"
#include "FrontendContext.h"
#include "ResourceMgr.h"
#include <Matrix3x4.h>
#include <Scene/Light.h>


#include <Qt3DRender/QGeometryRenderer>

namespace Nome
{

CInteractiveLight::CInteractiveLight(Scene::CSceneTreeNode* node)
    : SceneTreeNode(node)

{
    UpdateTransform();
    UpdateLight();
    InitInteractions();
}

void CInteractiveLight::UpdateTransform()
{
    if (!Transform)
    {
        Transform = new Qt3DCore::QTransform(this);
        this->addComponent(Transform);
    }
    const auto& tf = SceneTreeNode->L2WTransform.GetValue(tc::Matrix3x4::IDENTITY);
    QMatrix4x4 qtf { tf.ToMatrix4().Data() };
    Transform->setMatrix(qtf);
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
        auto* LightInstance = dynamic_cast<Scene::CLightInstance*>(entity);
        if (LightInstance)
        {


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


void CInteractiveLight::InitInteractions()
{
    // Only Light instances support vertex picking
    auto* Light = dynamic_cast<Scene::CLightInstance*>(SceneTreeNode->GetInstanceEntity());
    if (!Light)
        return;

    auto* picker = new Qt3DRender::QObjectPicker(this);
    picker->setHoverEnabled(true);
    connect(picker, &Qt3DRender::QObjectPicker::pressed, [](Qt3DRender::QPickEvent* pick) {
        if (pick->button() == Qt3DRender::QPickEvent::LeftButton)
        {
            const auto& wi = pick->worldIntersection();
            const auto& origin = GFrtCtx->NomeView->camera()->position();
            auto dir = wi - origin;

            tc::Ray ray({ origin.x(), origin.y(), origin.z() }, { dir.x(), dir.y(), dir.z() });

            if (GFrtCtx->NomeView->PickVertexBool)
                GFrtCtx->NomeView->PickVertexWorldRay(ray);
            if (GFrtCtx->NomeView->PickEdgeBool)
                GFrtCtx->NomeView->PickEdgeWorldRay(ray);
            if (GFrtCtx->NomeView->PickFaceBool)
                GFrtCtx->NomeView->PickFaceWorldRay(ray);
            if (GFrtCtx->NomeView->PickPolylineBool)
                GFrtCtx->NomeView->PickPolylineWorldRay(ray);
        }
    });
    this->addComponent(picker);
}


}