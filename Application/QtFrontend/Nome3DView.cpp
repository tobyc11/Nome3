#include "Nome3DView.h"
#include "FrontendContext.h"
#include "MainWindow.h"
#include <Scene/Mesh.h>

#include <QDialog>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStatusBar>
#include <QTableWidget>
#include <QBuffer>


namespace Nome
{

CNome3DView::CNome3DView()
    : mousePressEnabled(false)
    , animationEnabled(false)
    , rotationEnabled(true)
    , vertexSelectionEnabled(false)

{
    // Create a Base entity to host all entities
    Base = new Qt3DCore::QEntity;
    torus = new Qt3DCore::QEntity(Base);

    Root = new Qt3DCore::QEntity(Base);
    this->setRootEntity(Base);

    // Initialize the crystal ball
    auto *torusMesh = new Qt3DExtras::QTorusMesh;
    torusMesh->setRadius(1.0f);
    torusMesh->setMinorRadius(0.001f);
    torusMesh->setRings(100);
    torusMesh->setSlices(100);

    material = new Qt3DExtras::QPhongAlphaMaterial(Root);
    material->setAlpha(0.0f);
    material->setDiffuse(QColor(0, 255, 0));
    material->setAmbient(QColor(0, 255, 0));
    material->setShininess(5);


    torus->addComponent(torusMesh);
    torus->addComponent(material);




    // MakeGridEntity(Root); Removing grid entity per Professor Sequin's request

    // Make a point light
    auto* lightEntity = new Qt3DCore::QEntity(Base);
    auto* light = new Qt3DRender::QPointLight(lightEntity);
    light->setColor("white");
    light->setIntensity(1);
    lightEntity->addComponent(light);
    auto* lightTransform = new Qt3DCore::QTransform(lightEntity);
    lightTransform->setTranslation({ 100.0f, 100.0f, 100.0f });
    lightEntity->addComponent(lightTransform);

    // Tweak render settings
    this->defaultFrameGraph()->setClearColor(QColor(QRgb(0x4d4d4f)));

    // Setup camera
    zPos = 2.73;
    cameraset = this->camera();
    cameraset->lens()->setPerspectiveProjection(45.0f, 1280.f / 720.f, 0.1f, 1000.0f);
    cameraset->setPosition(QVector3D(0, 0, zPos));
    cameraset->setViewCenter(QVector3D(0, 0, 0));

    // Xinyu add on Oct 8 for rotation
    projection.setToIdentity();
    objectX = objectY = objectZ = 0;
    // Set up the animated rotation and activate by space key
    sphereTransform = new Qt3DCore::QTransform;
    controller = new OrbitTransformController(rotation, sphereTransform);
    controller->setTarget(sphereTransform);
    controller->setRadius(0);
    sphereRotateTransformAnimation = new QPropertyAnimation(sphereTransform);
    sphereRotateTransformAnimation->setTargetObject(controller);
    sphereRotateTransformAnimation->setPropertyName("angle");
    sphereRotateTransformAnimation->setStartValue(QVariant::fromValue(0));
    sphereRotateTransformAnimation->setEndValue(QVariant::fromValue(360));
    sphereRotateTransformAnimation->setDuration(100000);
    sphereRotateTransformAnimation->setLoopCount(-1);

    Root->addComponent(sphereTransform);

}

CNome3DView::~CNome3DView() { UnloadScene(); }

void CNome3DView::TakeScene(const tc::TAutoPtr<Scene::CScene>& scene)
{
    using namespace Scene;
    Scene = scene;
    Scene->Update();
    Scene->ForEachSceneTreeNode([this](CSceneTreeNode* node) {
        printf("%s\n", node->GetPath().c_str());
        auto* entity = node->GetInstanceEntity();
        if (!entity)
        {
            entity = node->GetOwner()->GetEntity();
        }

        if (entity)
        {
            printf("    %s\n", entity->GetName().c_str());

            // Create an InteractiveMesh from the scene node
            auto* mesh = new CInteractiveMesh(node);
            mesh->setParent(this->Root);
            InteractiveMeshes.insert(mesh);
        }
    });
    PostSceneUpdate();
}

void CNome3DView::UnloadScene()
{
    for (auto* m : InteractiveMeshes)
        delete m;
    InteractiveMeshes.clear();
    Scene = nullptr;
}


void CNome3DView::PostSceneUpdate()
{
    using namespace Scene;
    std::unordered_map<CSceneTreeNode*, CInteractiveMesh*> sceneNodeAssoc;
    std::unordered_set<CInteractiveMesh*> aliveSet;
    std::unordered_map<Scene::CEntity*, CDebugDraw*> aliveEntityDrawData;
    for (auto* m : InteractiveMeshes)
        sceneNodeAssoc.emplace(m->GetSceneTreeNode(), m);

    Scene->ForEachSceneTreeNode([&](CSceneTreeNode* node) {
        // Obtain either an instance entity or a shared entity from the scene node
        auto* entity = node->GetInstanceEntity();
        if (!entity)
        {
            entity = node->GetOwner()->GetEntity();
        }

        if (entity)
        {
            CInteractiveMesh* mesh = nullptr;
            // Check for existing InteractiveMesh
            auto iter = sceneNodeAssoc.find(node);
            if (iter != sceneNodeAssoc.end())
            {
                // Found existing InteractiveMesh, mark as alive
                mesh = iter->second;
                aliveSet.insert(mesh);
                mesh->UpdateTransform();
                if (node->WasEntityUpdated())
                {
                    printf("Geom regen for %s\n", node->GetPath().c_str());
                    mesh->UpdateGeometry(PickVertexBool);
                    mesh->UpdateMaterial(WireFrameMode);
                    node->SetEntityUpdated(false);
                }
            }
            else
            {
                mesh = new CInteractiveMesh(node);
                mesh->setParent(this->Root);
                aliveSet.insert(mesh);
                InteractiveMeshes.insert(mesh);
            }

            // Create a DebugDraw for the CEntity if not already
            auto eIter = EntityDrawData.find(entity);
            if (eIter == EntityDrawData.end())
            {
                auto* debugDraw = new CDebugDraw(Root);
                aliveEntityDrawData[entity] = debugDraw;
                // TODO: somehow uncommenting this line leads to a crash in Qt3D
                // mesh->SetDebugDraw(debugDraw);
            }
            else
            {
                aliveEntityDrawData[entity] = eIter->second;
                mesh->SetDebugDraw(eIter->second);
            }
        }
    });

    // Now kill all the dead objects, i.e., not longer in the scene graph. If it wasn't added to
    // aliveset above, then it is dead.
    for (auto* m : InteractiveMeshes)
    {
        auto iter = aliveSet.find(m);
        if (iter == aliveSet.end())
        {
            // Not in aliveSet
            delete m;
        }
    }
    InteractiveMeshes = std::move(aliveSet);

    // Kill all entity debug draws that are not alive
    for (auto& iter : EntityDrawData)
    {
        auto iter2 = aliveEntityDrawData.find(iter.first);
        if (iter2 == aliveEntityDrawData.end())
        {
            delete iter.second;
        }
    }

    EntityDrawData = std::move(aliveEntityDrawData);
    for (const auto& pair : EntityDrawData)
    {
        pair.second->Reset();
        pair.first->Draw(pair.second);
        pair.second->Commit();
    }
}

// Randy added 9/27
void CNome3DView::ClearSelectedVertices()
{
    SelectedVertices.clear();
    Scene->ForEachSceneTreeNode([&](Scene::CSceneTreeNode* node) {
        // Obtain either an instance entity or a shared entity from the scene node
        auto* entity = node->GetInstanceEntity();
        if (!entity)
            entity = node->GetOwner()->GetEntity();
        if (entity)
        {
            auto* meshInst = dynamic_cast<Scene::CMeshInstance*>(entity);
            meshInst->DeselectAll();
        }
    });
}

// Randy added on 10/14 to clear face selection
void CNome3DView::ClearSelectedFaces()
{
    SelectedFaces.clear();
    Scene->ForEachSceneTreeNode([&](Scene::CSceneTreeNode* node) {
        // Obtain either an instance entity or a shared entity from the scene node
        auto* entity = node->GetInstanceEntity();
        if (!entity)
            entity = node->GetOwner()->GetEntity();
        if (entity)
        {
            auto* meshInst = dynamic_cast<Scene::CMeshInstance*>(entity);
            meshInst->DeselectAll();
        }
    });
}

// Randy added on 11/5 to clear edge selection
void CNome3DView::ClearSelectedEdges()
{
    SelectedEdgeVertices.clear();
    Scene->ForEachSceneTreeNode([&](Scene::CSceneTreeNode* node) {
        // Obtain either an instance entity or a shared entity from the scene node
        auto* entity = node->GetInstanceEntity();
        if (!entity)
            entity = node->GetOwner()->GetEntity();
        if (entity)
        {
            auto* meshInst = dynamic_cast<Scene::CMeshInstance*>(entity);
            meshInst->DeselectAll();
        }
    });
}


void CNome3DView::PickFaceWorldRay(tc::Ray& ray)
{
    rotateRay(ray);
    std::vector<std::tuple<float, Scene::CMeshInstance*, std::string>> hits;
    Scene->ForEachSceneTreeNode([&](Scene::CSceneTreeNode* node) {
        // Obtain either an instance entity or a shared entity from the scene node
        auto* entity = node->GetInstanceEntity();
        if (!entity)
            entity = node->GetOwner()->GetEntity();
        if (entity)
        {
            const auto& l2w = node->L2WTransform.GetValue(tc::Matrix3x4::IDENTITY);
            auto localRay = ray.Transformed(l2w.Inverse());
            localRay.Direction =
                localRay.Direction
                    .Normalized(); // Normalize to fix "scale" error caused by l2w.Inverse()
            auto* meshInst = dynamic_cast<Scene::CMeshInstance*>(entity);
            auto pickResults = meshInst->PickFaces(localRay);
            for (const auto& [dist, name] : pickResults)
                hits.emplace_back(dist, meshInst, name);
        }
    });

    std::sort(hits.begin(), hits.end());
    // if (!hits.empty()) {
    //    hits.resize(1); // Force there to be only one face selected. This is more user-friendly.
    //}
    if (hits.size() == 1)
    {
        const auto& [dist, meshInst, faceName] = hits[0];
        std::vector<std::string>::iterator position =
            std::find(SelectedFaces.begin(), SelectedFaces.end(), faceName);
        if (position == SelectedFaces.end())
        { // if this face has not been selected before
            SelectedFaces.push_back(faceName); // add face to selected faces
            GFrtCtx->MainWindow->statusBar()->showMessage(
                QString::fromStdString("Selected " + faceName));
        }
        else // else, this face has been selected previously
        {
            SelectedFaces.erase(position);
            GFrtCtx->MainWindow->statusBar()->showMessage(
                QString::fromStdString("Deselected " + faceName));
        }
        meshInst->MarkFaceAsSelected({ faceName }, true);
    }
    else if (!hits.empty())
    {
        // Show a dialog for the user to choose one face
        auto* dialog = new QDialog(GFrtCtx->MainWindow);
        dialog->setModal(true);
        auto* layout1 = new QHBoxLayout(dialog);
        auto* table = new QTableWidget();
        table->setRowCount(hits.size());
        table->setColumnCount(2);
        QStringList titles;
        titles.append(QString::fromStdString("Closeness Rank"));
        titles.append(QString::fromStdString("Face Name"));
        table->setHorizontalHeaderLabels(titles);
        int closenessRank = 1;
        for (size_t i = 0; i < hits.size(); i++)
        {
            const auto& [dist, meshInst, faceName] = hits[i];
            if (i != 0)
            {
                const auto& [prevDist, prevMeshInst, prevFaceName] = hits[i - 1];
                if (round(dist * 100) != round(prevDist * 100))
                {
                    closenessRank += 1;
                }
                // else, closenessRank stay the same as prev as the distance is the same (faces
                // in same location)
            }

            auto* distWidget = new QTableWidgetItem(QString::number(closenessRank));
            auto* item = new QTableWidgetItem(QString::fromStdString(faceName));
            table->setItem(i, 0, distWidget); // i is row num, and 0 is col num
            table->setItem(i, 1, item);
        }
        layout1->addWidget(table);
        auto* layout2 = new QVBoxLayout();
        auto* btnOk = new QPushButton();
        btnOk->setText("OK");
        connect(btnOk, &QPushButton::clicked, [this, dialog, table, hits]() {
            auto sel = table->selectedItems();
            if (!sel.empty())
            {
                int row = sel[0]->row();
                const auto& [dist, meshInst, faceName] = hits[row];
                std::vector<std::string>::iterator position =
                    std::find(SelectedFaces.begin(), SelectedFaces.end(), faceName);
                if (position == SelectedFaces.end())
                { // if this face has not been selected before
                    SelectedFaces.push_back(faceName); // add face to selected face
                    GFrtCtx->MainWindow->statusBar()->showMessage(
                        QString::fromStdString("Selected " + faceName));
                }
                else // else, this face has been selected previously
                {
                    SelectedFaces.erase(position);
                    GFrtCtx->MainWindow->statusBar()->showMessage(
                        QString::fromStdString("Deselected " + faceName));
                }
                float selected_dist = round(dist * 100);

                // mark all those that share the same location
                for (int i = 0; i < hits.size(); i++)
                {
                    const auto& [dist, meshInst, overlapfaceName] = hits[i];
                    if (round(dist * 100) == selected_dist)
                    {
                        meshInst->MarkFaceAsSelected({ overlapfaceName }, true);
                    }
                }
            }
            dialog->close();
        });
        auto* btnCancel = new QPushButton();
        btnCancel->setText("Cancel");
        connect(btnCancel, &QPushButton::clicked, dialog, &QWidget::close);
        layout2->addWidget(btnOk);
        layout2->addWidget(btnCancel);
        layout1->addLayout(layout2);
        dialog->show();
    }
    else
    {
        GFrtCtx->MainWindow->statusBar()->showMessage("No face hit.");
    }
}

// Used for picking edges
void CNome3DView::PickEdgeWorldRay(tc::Ray& ray)
{
    rotateRay(ray);
    std::vector<std::tuple<float, Scene::CMeshInstance*, std::vector<std::string>>>
        hits; // note the string is a vector of strings
    Scene->ForEachSceneTreeNode([&](Scene::CSceneTreeNode* node) {
        std::cout << "Currently in NOME3DView's PickEdgeWorldRay. At node: " + node->GetPath()
                  << std::endl;
        // Obtain either an instance entity or a shared entity from the scene node
        auto* entity = node->GetInstanceEntity();
        if (!entity)
            entity = node->GetOwner()->GetEntity();
        if (entity)
        {
            const auto& l2w = node->L2WTransform.GetValue(tc::Matrix3x4::IDENTITY);
            auto localRay = ray.Transformed(l2w.Inverse());
            localRay.Direction =
                localRay.Direction
                    .Normalized(); // Normalize to fix "scale" error caused by l2w.Inverse()

            auto* meshInst = dynamic_cast<Scene::CMeshInstance*>(entity);
            auto pickResults = meshInst->PickEdges(localRay);

            for (const auto& [dist, names] : pickResults)
                hits.emplace_back(dist, meshInst, names);
        }
    });
    std::sort(hits.begin(), hits.end());

    // TODO 11/21, If it contains a temp SELECT EDGE polyline, then immediately return that one
    std::vector<std::tuple<float, Scene::CMeshInstance*, std::vector<std::string>>> temp;
    for (auto hit : hits)
    {
        auto [dist, meshInst, edgeVertNames] = hit;
        if (edgeVertNames[0].find("SELECTED") != std::string::npos)
        {
            temp.push_back(hit);
        }
    }

    if (!temp.empty())
        hits = temp;
    if (hits.size() == 1)
    {
        const auto& [dist, meshInst, edgeVertNames] =
            hits[0]; // where the edgeVertNames is defined to a vector of two vertex names
        std::vector<std::string>::iterator position1 =
            std::find(SelectedEdgeVertices.begin(), SelectedEdgeVertices.end(), edgeVertNames[0]);
        std::vector<std::string>::iterator position2 =
            std::find(SelectedEdgeVertices.begin(), SelectedEdgeVertices.end(), edgeVertNames[1]);
        SelectedEdgeVertices.push_back(edgeVertNames[0]);
        SelectedEdgeVertices.push_back(edgeVertNames[1]);

        // if the selected edge is not a SELECTED (temp) edge
        if (edgeVertNames[0].find("SELECTED") == std::string::npos)
            GFrtCtx->MainWindow->statusBar()->showMessage(QString::fromStdString(
                "Selected " + edgeVertNames[0] + edgeVertNames[1] + " edge"));
        else
        {
            GFrtCtx->MainWindow->statusBar()->showMessage("Deselected edge");
        }
        std::set<std::string> edgeVertNamesSet(edgeVertNames.begin(), edgeVertNames.end());
        meshInst->MarkEdgeAsSelected(edgeVertNamesSet, true); // here
    }
    // If you need to implement multiple edge intersection, please see the below line at:
    // https://github.com/randyfan/NOME3/commit/55ab6d81140d09f1725e261ed810c1a15646ab5c
    else
        GFrtCtx->MainWindow->statusBar()->showMessage(
            "No edge hit or more than one edge hit. Please select again");
}

void CNome3DView::PickVertexWorldRay(tc::Ray& ray)
{
    rotateRay(ray);
    std::vector<std::tuple<float, Scene::CMeshInstance*, std::string>> hits;
    Scene->ForEachSceneTreeNode([&](Scene::CSceneTreeNode* node) {
        // Obtain either an instance entity or a shared entity from the scene node
        auto* entity = node->GetInstanceEntity();
        if (!entity)
            entity = node->GetOwner()->GetEntity();
        if (entity)
        {
            const auto& l2w = node->L2WTransform.GetValue(tc::Matrix3x4::IDENTITY);
            auto localRay = ray.Transformed(l2w.Inverse());
            localRay.Direction =
                localRay.Direction
                    .Normalized(); // Normalize to fix "scale" error caused by l2w.Inverse()
            auto* meshInst = dynamic_cast<Scene::CMeshInstance*>(entity);
            auto pickResults = meshInst->PickVertices(localRay);
            for (const auto& [dist, name] : pickResults)
                hits.emplace_back(dist, meshInst, name);
        }
    });

    std::sort(hits.begin(), hits.end());

    if (hits.size() == 1)
    {
        const auto& [dist, meshInst, vertName] = hits[0];
        std::vector<std::string>::iterator position =
            std::find(SelectedVertices.begin(), SelectedVertices.end(), vertName);
        if (position == SelectedVertices.end()) // if this vertex has not been selected before
        {
            SelectedVertices.push_back(vertName); // add vertex to selected vertices
            GFrtCtx->MainWindow->statusBar()->showMessage(
                QString::fromStdString("Selected " + vertName));
        }
        else // else, this vertex has been selected previously
        {
            SelectedVertices.erase(position);
            GFrtCtx->MainWindow->statusBar()->showMessage(
                QString::fromStdString("Deselected " + vertName));
        }
        meshInst->MarkVertAsSelected({ vertName }, true);
    }
    else if (!hits.empty())
    {
        // Show a dialog for the user to choose one vertex
        auto* dialog = new QDialog(GFrtCtx->MainWindow);
        dialog->setModal(true);
        auto* layout1 = new QHBoxLayout(dialog);
        auto* table = new QTableWidget();
        table->setRowCount(hits.size());
        table->setColumnCount(2);
        QStringList titles;
        titles.append(QString::fromStdString("Closeness Rank"));
        titles.append(QString::fromStdString("Vertex Name"));
        table->setHorizontalHeaderLabels(titles);
        int closenessRank = 1;
        for (size_t i = 0; i < hits.size(); i++)
        {
            const auto& [dist, meshInst, vertName] = hits[i];
            if (i != 0)
            {
                const auto& [prevDist, prevMeshInst, prevVertName] = hits[i - 1];
                if (round(dist * 100) != round(prevDist * 100))
                {
                    closenessRank += 1;
                }
                // else, closenessRank stay the same as prev as the distance is the same (vertices
                // in same location)
            }

            auto* distWidget = new QTableWidgetItem(QString::number(closenessRank));
            auto* item = new QTableWidgetItem(QString::fromStdString(vertName));
            table->setItem(i, 0, distWidget); // i is row num, and 0 is col num
            table->setItem(i, 1, item);
        }
        layout1->addWidget(table);
        auto* layout2 = new QVBoxLayout();
        auto* btnOk = new QPushButton();
        btnOk->setText("OK");
        connect(btnOk, &QPushButton::clicked, [this, dialog, table, hits]() {
            auto sel = table->selectedItems();
            if (!sel.empty())
            {
                int row = sel[0]->row();
                const auto& [dist, meshInst, vertName] = hits[row];
                std::vector<std::string>::iterator position =
                    std::find(SelectedVertices.begin(), SelectedVertices.end(), vertName);
                if (position == SelectedVertices.end())
                { // if this vertex has not been selected before
                    SelectedVertices.push_back(vertName); // add vertex to selected vertices
                    GFrtCtx->MainWindow->statusBar()->showMessage(
                        QString::fromStdString("Selected " + vertName));
                }
                else // else, this vertex has been selected previously
                {
                    SelectedVertices.erase(position);
                    GFrtCtx->MainWindow->statusBar()->showMessage(
                        QString::fromStdString("Deselected " + vertName));
                }

                float selected_dist = round(dist * 100);

                // mark all those that share the same location
                for (int i = 0; i < hits.size(); i++)
                {
                    const auto& [dist, meshInst, overlapvertName] = hits[i];
                    if (round(dist * 100) == selected_dist)
                    {
                        meshInst->MarkVertAsSelected({ overlapvertName }, true);
                    }
                }
            }
            dialog->close();
        });
        auto* btnCancel = new QPushButton();
        btnCancel->setText("Cancel");
        connect(btnCancel, &QPushButton::clicked, dialog, &QWidget::close);
        layout2->addWidget(btnOk);
        layout2->addWidget(btnCancel);
        layout1->addLayout(layout2);
        dialog->show();
    }
    else
    {
        GFrtCtx->MainWindow->statusBar()->showMessage("No point hit.");
    }
}


// Currently not used
Qt3DCore::QEntity* CNome3DView::MakeGridEntity(Qt3DCore::QEntity* parent)
{
    const float size = 100.0f;
    const int divisions = 100;
    const float halfSize = size / 2.0f;

    auto* gridEntity = new Qt3DCore::QEntity(parent);

    auto* geometry = new Qt3DRender::QGeometry(gridEntity);

    QByteArray bufferBytes;
    bufferBytes.resize(3 * sizeof(float) * 4 * (divisions + 1));
    auto* positions = reinterpret_cast<float*>(bufferBytes.data());
    float xStart = -size / 2.0f;
    float increment = size / divisions;
    for (int i = 0; i <= divisions; i++)
    {
        *positions++ = xStart;
        *positions++ = 0.0f;
        *positions++ = -halfSize;
        *positions++ = xStart;
        *positions++ = 0.0f;
        *positions++ = halfSize;

        *positions++ = -halfSize;
        *positions++ = 0.0f;
        *positions++ = xStart;
        *positions++ = halfSize;
        *positions++ = 0.0f;
        *positions++ = xStart;
        xStart += increment;
    }

    auto* buf = new QBuffer((QByteArray*)Qt3DRender::QBuffer::VertexBuffer, geometry);
    buf->setData(bufferBytes);

    auto* positionAttr = new Qt3DRender::QAttribute(geometry);
    positionAttr->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());
    positionAttr->setVertexBaseType(Qt3DRender::QAttribute::Float);
    positionAttr->setVertexSize(3);
    positionAttr->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    positionAttr->setBuffer(reinterpret_cast<Qt3DRender::QBuffer*>(buf));
    positionAttr->setByteStride(3 * sizeof(float));
    positionAttr->setCount(4 * (divisions + 1));
    geometry->addAttribute(positionAttr);

    auto* gridMesh = new Qt3DRender::QGeometryRenderer(gridEntity);
    gridMesh->setGeometry(geometry);
    gridMesh->setPrimitiveType(Qt3DRender::QGeometryRenderer::Lines);
    auto* material = new Qt3DExtras::QPhongMaterial(gridEntity);
    material->setAmbient({ 255, 255, 255 });

    gridEntity->addComponent(gridMesh);
    gridEntity->addComponent(material);

    return gridEntity;
}


// Xinyu add on Oct 8 for rotation
void CNome3DView::mousePressEvent(QMouseEvent* e)
{
    if (!vertexSelectionEnabled)
    {
        material->setAlpha(0.7f);

        rotationEnabled = e->button() == Qt::RightButton ? false : true;
        zPos = cameraset->position().z();
        // Save mouse press position
        firstPosition = QVector2D(e->localPos());
        mousePressEnabled = true;
    }
}


void CNome3DView::mouseMoveEvent(QMouseEvent* e)
{
    if (mousePressEnabled) {
        // Mouse release position - mouse press position
        secondPosition = QVector2D(e->localPos());
        QVector2D diff = secondPosition - firstPosition;
        if (!rotationEnabled)
        {
            objectX = diff.x() / 100 + objectX;
            objectY = - diff.y() / 100 + objectY;
            sphereTransform->setTranslation(QVector3D(objectX, objectY, objectZ));

        } else {
            QVector2D firstPoint = GetProjectionPoint(firstPosition);
            QVector2D secondPoint = GetProjectionPoint(secondPosition);
            double projectedRadius = sqrt(qPow(zPos, 2) - 1) / zPos;

            if (firstPoint.length() > projectedRadius || secondPoint.length() > projectedRadius)
            {
                float angle =
                    qRadiansToDegrees(qAsin(
                        QVector3D::crossProduct(QVector3D(firstPoint, 0).normalized()
                                                    , QVector3D(secondPoint, 0).normalized()).z()));
                rotation = QQuaternion::fromAxisAndAngle(0, 0, 1, angle) * rotation;
            }
            else
            {
                QVector3D firstCrystalPoint = GetCrystalPoint(firstPoint);
                QVector3D secondCrystalPoint = GetCrystalPoint(secondPoint);
                QVector3D axis =
                    QVector3D::crossProduct(firstCrystalPoint, secondCrystalPoint).normalized();
                float distance = firstCrystalPoint.distanceToPoint(secondCrystalPoint);
                rotation =
                    QQuaternion::fromAxisAndAngle(axis, qRadiansToDegrees(2 * asin(distance / 2)))
                    * rotation;
            }
        }
        sphereTransform->setRotation(rotation);

        firstPosition = secondPosition;
    }
}

void CNome3DView::mouseReleaseEvent(QMouseEvent* e)
{
    material->setAlpha(0.0f);

    mousePressEnabled = false;
}

void CNome3DView::wheelEvent(QWheelEvent *ev)
{
    if (rotationEnabled)
    {
        QVector3D cameraPosition = cameraset->position();
        zPos = cameraPosition.z();
        QPoint numPixels = ev->pixelDelta();
        QPoint numDegrees = ev->angleDelta() / 10.0f;

        if (!numPixels.isNull())
        {
            objectZ += numPixels.y() * 0.2;
        }
        else if (!numDegrees.isNull())
        {
            QPoint numSteps = numDegrees / 15;
            objectZ += numSteps.y() * 0.2;
        }
        if (objectZ > 2)
            objectZ = 2;
        sphereTransform->setTranslation(QVector3D(objectX, objectY, objectZ));
        ev->accept();
    }
}

void CNome3DView::keyPressEvent(QKeyEvent *ev)
{
    switch (ev->key())
    {
    case Qt::Key_Tab:
        material->setAlpha(rotationEnabled * 0.1);

        break;
    case Qt::Key_Shift:
        vertexSelectionEnabled = true;
        break;
    case Qt::Key_Space:
        if (animationEnabled) {
            sphereRotateTransformAnimation->pause();
        }   else {
            sphereRotateTransformAnimation->start();
        }
        animationEnabled = !animationEnabled;
        break;
    }
}


bool CNome3DView::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyRelease) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Shift) {
            return true;
        }
        else
            return false;
    }
    return false;
}

QVector2D CNome3DView::GetProjectionPoint(QVector2D originalPosition) {
    double xRatio = (originalPosition.x() - this->width() / 2.0) / (this->width() / 2.0);
    double yRatio = (this->height() / 2.0 - originalPosition.y()) / (this->height() / 2.0);
    // Calculate the x ratio according to the screen ratio
    double tempX = xRatio * this->width() / this->height();
    // Calculate the equivalent y by the radius
    double tempY = sqrt(qPow(tempX, 2) + qPow(yRatio, 2));
    //Calculate the camera view angle according to the picked point
    double theta = qAtan(tempY * qTan(qDegreesToRadians(cameraset->lens()->fieldOfView() / 2)));

    double temp = 1 + qPow(qTan(theta), 2);
    double judge = (1 - qPow(zPos, 2)) / temp + qPow(zPos / temp, 2);

    double projectedHeight = (judge > 0 ? (zPos / temp - qSqrt(judge)) : (zPos - 1 / zPos))
        * qTan(qDegreesToRadians(cameraset->lens()->fieldOfView() / 2));

    double projectedWidth = projectedHeight * this->width() / this->height();

    return QVector2D(xRatio * projectedWidth, yRatio * projectedHeight);
}

QVector3D CNome3DView::GetCrystalPoint(QVector2D originalPoint) {
    double z = sqrt(1 - qPow(originalPoint.x(), 2) - qPow(originalPoint.y(), 2));
    return QVector3D(originalPoint, z);
}

void CNome3DView::rotateRay(tc::Ray& ray) {
    QVector3D origin = QVector3D(ray.Origin.x, ray.Origin.y, ray.Origin.z) - QVector3D(objectX, objectY, objectZ);;
    origin = rotation.inverted().rotatedVector(origin);
    QVector3D direction = rotation.inverted().rotatedVector(QVector3D(ray.Direction.x, ray.Direction.y, ray.Direction.z));

    ray.Direction = tc::Vector3(direction.x(), direction.y(), direction.z());
    ray.Origin = tc::Vector3(origin.x(), origin.y(), origin.z());
}
void CNome3DView::FreeVertexSelection() {
    vertexSelectionEnabled = false;
}

}