#include "InteractiveMesh.h"
#include "FrontendContext.h"
#include "MaterialParser.h"
#include "MeshToQGeometry.h"
#include "Nome3DView.h"
#include "ResourceMgr.h"

#include <Matrix3x4.h>
#include <Scene/Mesh.h>

#include <Qt3DExtras/QSphereMesh>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QObjectPicker>
#include <Qt3DRender/QPickEvent>

namespace Nome
{

CInteractiveMesh::CInteractiveMesh(Scene::CSceneTreeNode* node)
    : SceneTreeNode(node)
    , PointEntity {}
    , PointMaterial {}
    , PointGeometry {}
    , PointRenderer {}
{
    UpdateTransform();
    UpdateGeometry();
    UpdateMaterial();
    InitInteractions();
}

void CInteractiveMesh::UpdateTransform()
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
            delete GeometryRenderer;
            delete Geometry;

            CMeshToQGeometry meshToQGeometry(meshInstance->GetMeshImpl(), true);
            Geometry = meshToQGeometry.GetGeometry();
            Geometry->setParent(this);

            GeometryRenderer = new Qt3DRender::QGeometryRenderer(this);
            GeometryRenderer->setGeometry(Geometry);
            GeometryRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Triangles);
            this->addComponent(GeometryRenderer);

            // Update or create the entity for drawing vertices
            if (!PointEntity)
            {
                PointEntity = new Qt3DCore::QEntity(this);

                auto xmlPath = CResourceMgr::Get().Find("DebugDrawLine.xml");
                auto* lineMat = new CXMLMaterial(QString::fromStdString(xmlPath));
                PointMaterial = lineMat;
                PointMaterial->setParent(this);
                PointEntity->addComponent(PointMaterial);
            }
            else
            {
                delete PointRenderer;
                delete PointGeometry;
            }
            PointGeometry = meshToQGeometry.GetPointGeometry();
            PointGeometry->setParent(PointEntity);
            PointRenderer = new Qt3DRender::QGeometryRenderer(PointEntity);
            PointRenderer->setGeometry(PointGeometry);
            PointRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Points);
            PointEntity->addComponent(PointRenderer);
        }
        else
        {
            // The entity is not a mesh instance, we don't know how to handle it
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
    QVector3D instanceColor { 1.0f, 0.5f, 0.1f };
    if (auto surface = SceneTreeNode->GetOwner()->GetSurface())
    {
        instanceColor.setX(surface->ColorR.GetValue(1.0f));
        instanceColor.setY(surface->ColorG.GetValue(1.0f));
        instanceColor.setZ(surface->ColorB.GetValue(1.0f));
    }

    if (!Material)
    {
        auto xmlPath = CResourceMgr::Get().Find("WireframeLit.xml");
        auto* mat = new CXMLMaterial(QString::fromStdString(xmlPath));
        this->addComponent(mat);
        Material = mat;
    }
    auto* mat = dynamic_cast<CXMLMaterial*>(Material);
    mat->FindParameterByName("kd")->setValue(instanceColor);

    // Use non-default line color only if the instance has a surface
    auto surface = SceneTreeNode->GetOwner()->GetSurface();
    if (LineMaterial && surface)
    {
        auto* lineMat = dynamic_cast<CXMLMaterial*>(LineMaterial);
        lineMat->FindParameterByName("instanceColor")->setValue(instanceColor);
    }
}

void CInteractiveMesh::InitInteractions()
{
    // Only mesh instances support vertex picking
    auto* mesh = dynamic_cast<Scene::CMeshInstance*>(SceneTreeNode->GetInstanceEntity());
    if (!mesh)
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
            bool additive = pick->modifiers() & Qt::ShiftModifier;
            GFrtCtx->NomeView->PickVertexWorldRay(ray, additive);
        }
    });
    this->addComponent(picker);
}

void CInteractiveMesh::SetDebugDraw(const CDebugDraw* debugDraw)
{
    // Check for existing lineEntity and delete
    auto* oldEntity = this->findChild<Qt3DCore::QEntity*>(QStringLiteral("lineEntity"));
    if (oldEntity)
    {
        auto* oldRenderer = oldEntity->findChild<Qt3DRender::QGeometryRenderer*>();
        if (oldRenderer && oldRenderer->geometry() == debugDraw->GetLineGeometry())
            return; // No work to be done if geometry stays the same
    }
    delete oldEntity;

    // printf("this=%p debugDraw=%p\n", this, debugDraw);

    auto* lineEntity = new Qt3DCore::QEntity(this);
    lineEntity->setObjectName(QStringLiteral("lineEntity"));

    if (!LineMaterial)
    {
        auto xmlPath = CResourceMgr::Get().Find("DebugDrawLine.xml");
        auto* lineMat = new CXMLMaterial(QString::fromStdString(xmlPath));
        LineMaterial = lineMat;
        LineMaterial->setObjectName(QStringLiteral("lineMaterial"));
        LineMaterial->setParent(this);
        lineEntity->addComponent(LineMaterial);

        if (auto surface = SceneTreeNode->GetOwner()->GetSurface())
        {
            QVector3D instanceColor;
            instanceColor.setX(surface->ColorR.GetValue(1.0f));
            instanceColor.setY(surface->ColorG.GetValue(1.0f));
            instanceColor.setZ(surface->ColorB.GetValue(1.0f));
            lineMat->FindParameterByName("instanceColor")->setValue(instanceColor);
        }
    }

    auto* lineRenderer = new Qt3DRender::QGeometryRenderer(lineEntity);
    lineRenderer->setGeometry(debugDraw->GetLineGeometry());
    lineRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Lines);
    lineEntity->addComponent(lineRenderer);
}

}
