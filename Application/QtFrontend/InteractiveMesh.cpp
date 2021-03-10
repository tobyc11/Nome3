#include "InteractiveMesh.h"
#include "FrontendContext.h"
#include "MaterialParser.h"
#include "Nome3DView.h"
#include "ResourceMgr.h"
#include <Matrix3x4.h>
#include <Scene/Mesh.h>

#include "DataStructureMeshToQGeometry.h" // Project ChangeDS

#include <Qt3DExtras/QSphereMesh>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QObjectPicker>
#include <Qt3DRender/QPickEvent>

namespace Nome
{

    using namespace Qt3DRender;

CInteractiveMesh::CInteractiveMesh(Scene::CSceneTreeNode* node)
    : SceneTreeNode(node)
    , PointEntity {}
    , PointMaterial {}
    , PointGeometry {}
    , PointRenderer {}
    , InstanceColor {95.0 / 255.0, 75.0 / 255.0, 139.0 / 255.0}
{
    UpdateTransform();
    UpdateMaterial(false); // false = don't show facets by default
    UpdateGeometry(false); // false = don't show vert boxes by default
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
            if (PointRenderer)
                delete PointRenderer;
            if (PointGeometry)
                delete PointGeometry;

            // A Qt3DRender::QGeometry class is used to group a list of Qt3DRender::QAttribute
            // objects together to form a geometric shape Qt3D is able to render using
            // Qt3DRender::QGeometryRenderer.
            auto test = meshInstance->GetDSMesh();
            // TODO: replace with a better method
            auto selectedfacehandles =
                    meshInstance->GetSelectedFaceHandles(); // Randy added on 12/3

            auto DSFaceWithColorVector = meshInstance->GetDSFaceWithColorVector();

            CDataStructureMeshToQGeometry DSmeshToQGeometry(meshInstance->GetDSMesh(), InstanceColor,
                                                            true); // Project SwitchDS

            // Geometry = meshToQGeometry.GetGeometry();
            Geometry = DSmeshToQGeometry.GetGeometry();
            Geometry->setParent(this);
            GeometryRenderer = new Qt3DRender::QGeometryRenderer(this);
            GeometryRenderer->setGeometry(Geometry);
            GeometryRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Triangles);
            this->addComponent(GeometryRenderer); // adding geometry data to interactive mesh

            if (showVertBox) {
                /// TODO: alow the debug draw
                std::string xmlPath = "";
                if (!showVertBox)
                    xmlPath = CResourceMgr::Get().Find("DebugDrawLine.xml");
                else
                    xmlPath = CResourceMgr::Get().Find("DebugDrawLineWITHVERTBOX.xml");

                // May need to optimize this in the future. Cause we're parsing the file everytime the node is marked dirty, even though we could keep the material the same if that was not changed
                PointEntity = new Qt3DCore::QEntity(this);
                auto *lineMat = new CXMLMaterial(QString::fromStdString(xmlPath));
                PointMaterial = lineMat;
                PointMaterial->setParent(this);
                PointEntity->addComponent(PointMaterial);

                // PointGeometry = meshToQGeometry.GetPointGeometry();
                PointGeometry = DSmeshToQGeometry.GetPointGeometry();
                PointGeometry->setParent(PointEntity);
                PointRenderer = new Qt3DRender::QGeometryRenderer(PointEntity);
                PointRenderer->setGeometry(PointGeometry);
                PointRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Points);
                PointEntity->addComponent(PointRenderer);
            }

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
    // If the scene tree node is not within a group, then we can directly use its surface color, if it has one
    if (!SceneTreeNode->GetParent()->GetOwner()->IsGroup())
    {
        if (auto surface = SceneTreeNode->GetOwner()->GetSurface())
        {
            InstanceColor[0] = (surface->ColorR.GetValue(1.0f));
            InstanceColor[1] = (surface->ColorG.GetValue(1.0f));
            InstanceColor[2] = (surface->ColorB.GetValue(1.0f));
        }
    }
    else // else, the scenetreenode is within a group, and we keep bubbling up from where we are
         // (going up the tree) UNTIL we get to an instance scene node that has a surface color
        // To clarify, from a hierarchical standpoint, the color that is in the lowest node in the hierarchical tree representing the NOME file
           // will color the object in the scene.
    {
        bool setColor = false;
        auto currNode = SceneTreeNode;
        while (currNode->GetParent()->GetOwner()->IsGroup())
        { // while currNode is within a group
            if (auto surface = currNode->GetOwner()->GetSurface())
            { // if the currNode itself is assigned a surface color, then this color is prioritzed.
              // we set the color and break.
                InstanceColor[0] = (surface->ColorR.GetValue(1.0f));
                InstanceColor[1] = (surface->ColorG.GetValue(1.0f));
                InstanceColor[2] = (surface->ColorB.GetValue(1.0f));
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
                InstanceColor[0] = (surface->ColorR.GetValue(1.0f));
                InstanceColor[1] = (surface->ColorG.GetValue(1.0f));
                InstanceColor[2] = (surface->ColorB.GetValue(1.0f));
            }
        }
    }

    if (!Material)
    {
        /*
        auto xmlPath = CResourceMgr::Get().Find("WireframeLit.xml");
        auto* mat = new CXMLMaterial(QString::fromStdString(xmlPath));
         */
        // TO replace the material with qt library material to get better performance
        auto* mat  = new Qt3DExtras::QPerVertexColorMaterial;
        this->addComponent(mat);
        Material = mat;
    }
    for (auto light : GFrtCtx->NomeView->InteractiveLights) {
        if (light->type != AmbientLight) {
            this->addComponent(light->Light);
        } else {
            InstanceColor[0] = light->Color.redF();
            InstanceColor[1] = light->Color.greenF();
            InstanceColor[2] = light->Color.blueF();
        }
    }

    // Use non-default line color only if the instance has a surface
    auto surface = SceneTreeNode->GetOwner()->GetSurface();
    if (LineMaterial && surface)
    {
        auto* lineMat = dynamic_cast<CXMLMaterial*>(LineMaterial);
        lineMat->FindParameterByName("instanceColor")->setValue(QVector3D(InstanceColor[0], InstanceColor[1], InstanceColor[2]));
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
                GFrtCtx->NomeView->PickEdgeWorldRay(ray);
            if (GFrtCtx->NomeView->PickFaceBool)
                GFrtCtx->NomeView->PickFaceWorldRay(ray);
            if (GFrtCtx->NomeView->PickPolylineBool)
                GFrtCtx->NomeView->PickPolylineWorldRay(ray);
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
    QVector3D instanceColor { 1.0f, 0.0f, 1.0f }; // Magenta color is default
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
        QVector3D instanceColor;

        if (SceneTreeNode->GetOwner()->isSelected() && !SceneTreeNode->GetOwner()->isResetColor())
        {
            auto color = SceneTreeNode->GetOwner()->GetSelectSurface();
            instanceColor.setX(color.x);
            instanceColor.setY(color.y);
            instanceColor.setZ(color.z);
            SceneTreeNode->GetOwner()->DoneSelecting(); // set SelectBool to false
            SceneTreeNode->GetOwner()->NeedResetColor(); // next selection will reset
        }
        else if (!SceneTreeNode->GetParent()
                      ->GetOwner()
                      ->IsGroup()) // If the scene tree node is not within a group, then we can
                                   // directly use its surface color
        {
            if (auto surface = SceneTreeNode->GetOwner()->GetSurface())
            {
                instanceColor.setX(surface->ColorR.GetValue(1.0f));
                instanceColor.setY(surface->ColorG.GetValue(1.0f));
                instanceColor.setZ(surface->ColorB.GetValue(1.0f));
            }
            if (SceneTreeNode->GetOwner()->isResetColor())
            {
                SceneTreeNode->GetOwner()->DoneSelecting(); // set SelectBool to false
                SceneTreeNode->GetOwner()->DoneResettingColor();
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
                { // if the currNode itself is assigned a surface color, then this color is
                  // prioritzed. we set the color and break.
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
                currNode =
                    currNode->GetParent(); // then, here, currNode's parent is guaranteed to be a
                                           // instance scene tree node due to previous while loop
                if (auto surface = currNode->GetOwner()->GetSurface())
                {
                    instanceColor.setX(surface->ColorR.GetValue(1.0f));
                    instanceColor.setY(surface->ColorG.GetValue(1.0f));
                    instanceColor.setZ(surface->ColorB.GetValue(1.0f));
                }
            }
            if (SceneTreeNode->GetOwner()->isResetColor())
            {
                SceneTreeNode->GetOwner()->DoneSelecting(); // set SelectBool to false
                SceneTreeNode->GetOwner()->DoneResettingColor();
            }
        }
        lineMat->FindParameterByName("instanceColor")->setValue(instanceColor);
    }

    auto* lineRenderer = new Qt3DRender::QGeometryRenderer(lineEntity);
    lineRenderer->setGeometry(debugDraw->GetLineGeometry());
    lineRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Lines);
    lineEntity->addComponent(lineRenderer);
}

}