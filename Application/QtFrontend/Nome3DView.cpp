#include "Nome3DView.h"

namespace Nome
{

CNome3DView::CNome3DView()
{
    Root = new Qt3DCore::QEntity();
    this->setRootEntity(Root);
    MakeGridEntity(Root);

    //Make a point light
    auto* lightEntity = new Qt3DCore::QEntity(Root);
    auto* light = new Qt3DRender::QPointLight(lightEntity);
    light->setColor("white");
    light->setIntensity(1);
    lightEntity->addComponent(light);
    auto* lightTransform = new Qt3DCore::QTransform(lightEntity);
    lightTransform->setTranslation({100.0f, 100.0f, 100.0f});
    lightEntity->addComponent(lightTransform);

    //Tweak render settings
    this->defaultFrameGraph()->setClearColor(QColor(QRgb(0x4d4d4f)));

    //Setup camera
    //TODO: aspect ratio
    auto* camera = this->camera();
    camera->lens()->setPerspectiveProjection(45.0f, 1280.f / 720.f, 0.1f, 1000.0f);
    camera->setPosition(QVector3D(0, 0, 40.0f));
    camera->setViewCenter(QVector3D(0, 0, 0));

    auto* camController = new Qt3DExtras::QOrbitCameraController(Root);
    camController->setLinearSpeed(50.0f);
    camController->setLookSpeed(180.0f);
    camController->setCamera(camera);
}

CNome3DView::~CNome3DView()
{
    UnloadScene();
}

void CNome3DView::TakeScene(const tc::TAutoPtr<Scene::CScene>& Scene)
{
    using namespace Scene;

    //TODO: figure out when to update scene
    Scene->Update();
    Scene->ForEachSceneTreeNode([this](CSceneTreeNode* node)
                                {
                                    printf("%s\n", node->GetPath().c_str());
                                    auto* entity = node->GetInstanceEntity();
                                    if (!entity)
                                    {
                                        entity = node->GetOwner()->GetEntity();
                                    }

                                    if (entity)
                                    {
                                        printf("    %s\n", entity->GetName().c_str());

                                        auto* mesh = new CInteractiveMesh(node);
                                        mesh->setParent(this->Root);
                                        InteractiveMeshes.insert(mesh);
                                    }
                                });
}

void CNome3DView::UnloadScene()
{
    for (auto* m : InteractiveMeshes)
        delete m;
    InteractiveMeshes.clear();
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
    material->setAmbient({255, 255, 255});

    gridEntity->addComponent(gridMesh);
    gridEntity->addComponent(material);

    return gridEntity;
}

}
