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
    UpdateGeometry(false); // don't show vert boxes by default
    UpdateMaterial(false); // don't show facets by default
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

void CInteractiveMesh::UpdateGeometry(bool showVertBox)
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
            // A Qt3DRender::QGeometry class is used to group a list of Qt3DRender::QAttribute
            // objects together to form a geometric shape Qt3D is able to render using
            // Qt3DRender::QGeometryRenderer.
            auto selectedfacehandles =
                meshInstance->GetSelectedFaceHandles(); // Randy added on 12/3
            CMeshToQGeometry meshToQGeometry(meshInstance->GetMeshImpl(), selectedfacehandles,
                                             true); // Randy added 2nd argument on 12/3
            Geometry = meshToQGeometry.GetGeometry();
            Geometry->setParent(this);
            GeometryRenderer = new Qt3DRender::QGeometryRenderer(this);
            GeometryRenderer->setGeometry(Geometry);
            GeometryRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Triangles);
            this->addComponent(GeometryRenderer); // adding geometry data to interactive mesh

            std::string xmlPath = "";
            if (!showVertBox)
                xmlPath = CResourceMgr::Get().Find("DebugDrawLine.xml");
            else
                xmlPath = CResourceMgr::Get().Find("DebugDrawLineWITHVERTBOX.xml");

            // May need to optimize this in the future. Cause we're parsing the file everytime the
            // node is marked dirty, even though we could keep the material the same if that was not
            // changed
            PointEntity = new Qt3DCore::QEntity(this);
            auto* lineMat = new CXMLMaterial(QString::fromStdString(xmlPath));
            PointMaterial = lineMat;
            PointMaterial->setParent(this);
            PointEntity->addComponent(PointMaterial);

            delete PointRenderer;
            delete PointGeometry;

            PointGeometry = meshToQGeometry.GetPointGeometry();
            PointGeometry->setParent(PointEntity);
            PointRenderer = new Qt3DRender::QGeometryRenderer(PointEntity);
            PointRenderer->setGeometry(PointGeometry);
            PointRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Points);
            PointEntity->addComponent(PointRenderer);
        }
        else
        {
            std::cout << "The entity is not a mesh instance, we don't know how to handle it. For "
                         "example, if you try to instanciate a face, it'll generate this "
                         "placeholder sphere."
                      << std::endl;
            auto* vPlaceholder = new Qt3DExtras::QSphereMesh(this);
            vPlaceholder->setRadius(1.0f);
            vPlaceholder->setRings(16);
            vPlaceholder->setSlices(16);
            this->addComponent(vPlaceholder);
        }
    }
}

void CInteractiveMesh::UpdateMaterial(bool showFacets)
{
    QVector3D instanceColor { 1.0f, 0.5f, 0.1f }; // orange color

    // If the scene tree node is not within a group, then we can directly use its surface color
    if (!SceneTreeNode->GetParent()->GetOwner()->IsGroup())
    {
        if (auto surface = SceneTreeNode->GetOwner()->GetSurface())
        {
            instanceColor.setX(surface->ColorR.GetValue(1.0f));
            instanceColor.setY(surface->ColorG.GetValue(1.0f));
            instanceColor.setZ(surface->ColorB.GetValue(1.0f));
        }
    }
    else // else, the scenetreenode is within a group, and we keep bubbling up from where we are
         // (going up the tree) until we get to an instance scene node that has a surface color
    {
        bool setColor = false;
        auto currNode = SceneTreeNode;
        while (currNode->GetParent()->GetOwner()->IsGroup())
        { // while currNode is within a group
            if (auto surface = currNode->GetOwner()->GetSurface())
            { // if the currNode itself is assigned a surface color, then this color is prioritzed.
              // we set the color and break.
                instanceColor.setX(surface->ColorR.GetValue(1.0f));
                instanceColor.setY(surface->ColorG.GetValue(1.0f));
                instanceColor.setZ(surface->ColorB.GetValue(1.0f));
                setColor = true;
                break;
            }
            currNode = currNode->GetParent();
        }

        if (!setColor) // If the surface color hasn't been set yet
        {
            currNode = currNode->GetParent(); // here, currNode's parent is guaranteed to be a
                                              // instance scene tree node due to previous while loop

            if (auto surface = currNode->GetOwner()->GetSurface())
            {
                instanceColor.setX(surface->ColorR.GetValue(1.0f));
                instanceColor.setY(surface->ColorG.GetValue(1.0f));
                instanceColor.setZ(surface->ColorB.GetValue(1.0f));
            }
        }
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
    if (showFacets)
        mat->FindParameterByName("showFacets")->setValue(1);
    else
        mat->FindParameterByName("showFacets")->setValue(0);

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

            if (GFrtCtx->NomeView->PickVertexBool)
                GFrtCtx->NomeView->PickVertexWorldRay(ray);
            if (GFrtCtx->NomeView->PickEdgeBool)
                GFrtCtx->NomeView->PickEdgeWorldRay(ray); // Randy added on 10/29 for edge selection
            if (GFrtCtx->NomeView->PickFaceBool)
                GFrtCtx->NomeView->PickFaceWorldRay(
                    ray); // Randy added on 10/10 for face selection.
        }
    });
    this->addComponent(picker);
}

void CInteractiveMesh::SetDebugDraw(const CDebugDraw* debugDraw)
{
    // Check for existing lineEntity and delete
    auto* oldEntity = this->findChild<Qt3DCore::QEntity*>(QStringLiteral("lineEntity"));
    if (oldEntity
        && !SceneTreeNode->GetOwner()->isSelected()) // Randy added the second boolean on 11/21
    {
        auto* oldRenderer = oldEntity->findChild<Qt3DRender::QGeometryRenderer*>();
        if (oldRenderer && oldRenderer->geometry() == debugDraw->GetLineGeometry())
            return; // No work to be done if geometry stays the same
    }
    delete oldEntity;

    // printf("this=%p debugDraw=%p\n", this, debugDraw);

    auto* lineEntity = new Qt3DCore::QEntity(this);
    lineEntity->setObjectName(QStringLiteral("lineEntity"));

    if (!LineMaterial
        || SceneTreeNode->GetOwner()
               ->isSelected()) // Randy added the second boolean on 11/21 to color polyline/bspline
    {
        auto xmlPath = CResourceMgr::Get().Find("DebugDrawLine.xml");
        auto* lineMat = new CXMLMaterial(QString::fromStdString(xmlPath));
        LineMaterial = lineMat;
        LineMaterial->setObjectName(QStringLiteral("lineMaterial"));
        LineMaterial->setParent(this);
        lineEntity->addComponent(LineMaterial);
        // Randy added this on 11/21
        if (SceneTreeNode->GetOwner()->isSelected())
        {
            std::cout << "You have selected a polyline/bspline entity" << std::endl;
            QVector3D instanceColor;
            auto color = SceneTreeNode->GetOwner()->GetSelectSurface();
            std::cout << color.x + color.y + color.z << std::endl;
            instanceColor.setX(color.x);
            instanceColor.setY(color.y);
            instanceColor.setZ(color.z);
            lineMat->FindParameterByName("instanceColor")->setValue(instanceColor);
            SceneTreeNode->GetOwner()
                ->UnselectNode(); // deselect it so it won't be colored again the next time
        }
        else if (auto surface = SceneTreeNode->GetOwner()->GetSurface())
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