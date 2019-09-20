#include "InteractiveMesh.h"
#include "MeshToQGeometry.h"

#include <Scene/Mesh.h>
#include <Matrix3x4.h>

#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QSphereMesh>

namespace Nome
{

CInteractiveMesh::CInteractiveMesh(Scene::CSceneTreeNode* node) : SceneTreeNode(node)
{
    UpdateTransform();
    UpdateGeometry();
    UpdateMaterial();
}

void CInteractiveMesh::UpdateTransform()
{
    if (!Transform)
    {
        Transform = new Qt3DCore::QTransform(this);
        this->addComponent(Transform);
    }
    const auto& tf = SceneTreeNode->L2WTransform.GetValue(tc::Matrix3x4::IDENTITY);
    QMatrix4x4 qtf{tf.ToMatrix4().Data()};
    Transform->setMatrix(qtf);
}

void CInteractiveMesh::UpdateGeometry()
{
    auto* entity = SceneTreeNode->GetInstanceEntity();
    if (!entity)
    {
        entity = SceneTreeNode->GetOwner()->GetEntity();
    }

    if (entity)
    {
        auto* meshInstance = dynamic_cast<Scene::CMeshInstance*>(entity);
        if (meshInstance)
        {
            CMeshToQGeometry meshToQGeometry(meshInstance->GetMeshImpl());
            auto* vGeometry = meshToQGeometry.GetGeometry();
            vGeometry->setParent(this);

            auto* vGeomRenderer = new Qt3DRender::QGeometryRenderer(this);
            vGeomRenderer->setGeometry(vGeometry);
            vGeomRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Triangles);
            this->addComponent(vGeomRenderer);
        }
        else
        {
            auto* vPlaceholder = new Qt3DExtras::QSphereMesh(this);
            vPlaceholder->setRadius(1.0f);
            vPlaceholder->setRings(16);
            vPlaceholder->setSlices(16);
            this->addComponent(vPlaceholder);
        }
    }
}

void CInteractiveMesh::UpdateMaterial()
{
    if (!Material)
    {
        auto* mat = new Qt3DExtras::QPhongMaterial(this);
        mat->setAmbient({255, 127, 127});
        this->addComponent(mat);
        Material = mat;
    }
}

}
