#include "InteractivePoint.h"

//TODO: SERIOUS BREACH OF ABSTRACTION (or is it?)
#include <Render/Geometry.h>

namespace Nome
{

uint32_t CPickingManager::RegisterObj(CInteractivePoint* interactivePoint)
{
    PointMap[NextObjId] = interactivePoint;
    interactivePoint->ObjId = NextObjId;
    return NextObjId++;
}

void CPickingManager::UnregisterObj(uint32_t id)
{
    PointMap.erase(id);
}

CInteractivePoint* CPickingManager::Pick(uint32_t x, uint32_t y)
{
    if (PointMap.size() == 0)
        return nullptr;

    //Collect all points into a buffer
    //TODO: cache this
    struct PointAttrs
    {
        tc::Vector3 Pos;
        tc::Vector3 Color;
        uint32_t Id;
    };
    std::vector<PointAttrs> pointBuffer;
    pointBuffer.reserve(PointMap.size());
    for (const auto& pair : PointMap)
    {
        //TODO: should we pass on color here? pair.second->GetColor()
        PointAttrs attrs = { pair.second->GetPosition(), Vector3::ZERO, pair.first };
        pointBuffer.push_back(attrs);
    }
    CVertexBuffer pointBufferGPU(pointBuffer.size() * sizeof(PointAttrs), pointBuffer.data());

    //Render the points buffer with scene view parameters, Read back texture
    uint32_t id = GRenderer->RenderPickingPointsGetId(Scene->GetLastRenderViewInfo(), &pointBufferGPU, x, y);

    auto iter = PointMap.find(id);
    if (iter != PointMap.end())
        return iter->second;
    return nullptr;
}

std::string CInteractivePoint::GetName() const
{
    return Name;
}

void CInteractivePoint::SetName(std::string value)
{
    Name = value;
}

tc::Color CInteractivePoint::GetColor() const
{
    return Color;
}

void CInteractivePoint::SetColor(tc::Color value)
{
    Color = std::move(value);
}

tc::Vector3 CInteractivePoint::GetPosition() const
{
    return Position;
}

void CInteractivePoint::SetPosition(tc::Vector3 value)
{
    Position = std::move(value);
}

} /* namespace Nome */
