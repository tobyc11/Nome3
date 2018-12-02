#include "DocumentBuilder.h"

#include "Scene/Point.h"

namespace Nome
{

Scene::CPoint* CDocumentBuilder::Point(float x, float y, float z)
{
    auto* point = new Scene::CPoint(Document, "");
    point->SetDefaultPosition(x, y, z);
    return nullptr;
}

}
