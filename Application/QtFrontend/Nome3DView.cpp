#include "Nome3DView.h"
#include "FrontendContext.h"
#include "MainWindow.h"
#include <Scene/Mesh.h>

#include <QDialog>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStatusBar>
#include <QTableWidget>

namespace Nome
{

CNome3DView::CNome3DView()
{
    Root = new Qt3DCore::QEntity();
    this->setRootEntity(Root);
    MakeGridEntity(Root);

    // Make a point light
    auto* lightEntity = new Qt3DCore::QEntity(Root);
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
    // TODO: aspect ratio
    auto* camera = this->camera();
    camera->lens()->setPerspectiveProjection(45.0f, 1280.f / 720.f, 0.1f, 1000.0f);
    camera->setPosition(QVector3D(0, 0, 40.0f));
    camera->setViewCenter(QVector3D(0, 0, 0));

    auto* camController = new Qt3DExtras::QOrbitCameraController(Root);
    camController->setLinearSpeed(50.0f);
    camController->setLookSpeed(180.0f);
    camController->setCamera(camera);
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
                    mesh->UpdateGeometry();
                    mesh->UpdateMaterial();
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

    // Now kill all the dead objects, i.e., not longer in the scene graph
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

void CNome3DView::PickVertexWorldRay(const tc::Ray& ray, bool additive)
{
    if (!additive)
        SelectedVertices.clear();

    std::vector<std::pair<float, std::string>> hits;
    Scene->ForEachSceneTreeNode([&](Scene::CSceneTreeNode* node) {
        // Obtain either an instance entity or a shared entity from the scene node
        auto* entity = node->GetInstanceEntity();
        if (!entity)
            entity = node->GetOwner()->GetEntity();
        if (entity)
        {
            const auto& l2w = node->L2WTransform.GetValue(tc::Matrix3x4::IDENTITY);
            auto localRay = ray.Transformed(l2w.Inverse());

            auto* meshInst = dynamic_cast<Scene::CMeshInstance*>(entity);
            auto pickResults = meshInst->PickVertices(localRay);
            hits.insert(hits.end(), pickResults.begin(), pickResults.end());
        }
    });

    std::sort(hits.begin(), hits.end());
    if (hits.size() == 1)
    {
        SelectedVertices.push_back(hits[0].second);
        GFrtCtx->MainWindow->statusBar()->showMessage(
            QString::fromStdString("Selected " + hits[0].second));
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
        for (size_t i = 0; i < hits.size(); i++)
        {
            auto* dist = new QTableWidgetItem(QString::number(hits[i].first));
            auto* item = new QTableWidgetItem(QString::fromStdString(hits[i].second));
            table->setItem(i, 0, dist);
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
                SelectedVertices.push_back(hits[row].second);
                GFrtCtx->MainWindow->statusBar()->showMessage(
                    QString::fromStdString("Selected " + hits[row].second));
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

    auto* buf = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::VertexBuffer, geometry);
    buf->setData(bufferBytes);

    auto* positionAttr = new Qt3DRender::QAttribute(geometry);
    positionAttr->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());
    positionAttr->setVertexBaseType(Qt3DRender::QAttribute::Float);
    positionAttr->setVertexSize(3);
    positionAttr->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    positionAttr->setBuffer(buf);
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

}
