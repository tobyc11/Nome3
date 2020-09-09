#include "InteractivePoint.h"

namespace Nome
{

uint32_t CPickingManager::RegisterObj(CInteractivePoint* interactivePoint)
{
    PointMap[NextObjId] = interactivePoint;
    interactivePoint->ObjId = NextObjId;
    return NextObjId++;
}

void CPickingManager::UnregisterObj(uint32_t id) { PointMap.erase(id); }

CInteractivePoint* CPickingManager::Pick(uint32_t x, uint32_t y)
{
    if (PointMap.empty())
        return nullptr;
    return nullptr;
}

std::string CInteractivePoint::GetName() const { return Name; }

void CInteractivePoint::SetName(std::string value) { Name = value; }

tc::Color CInteractivePoint::GetColor() const { return Color; }

void CInteractivePoint::SetColor(tc::Color value) { Color = std::move(value); }

tc::Vector3 CInteractivePoint::GetPosition() const { return Position; }

void CInteractivePoint::SetPosition(tc::Vector3 value) { Position = std::move(value); }

} /* namespace Nome */
