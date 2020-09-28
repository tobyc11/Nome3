#pragma once
#include "Entity.h"

namespace Nome::Scene
{

struct CSurfaceInfo {

    std::string surfacename;
    Vector3 Color;

};

class CSurface : public CEntity
{
    DEFINE_INPUT(float, ColorR) {}
    DEFINE_INPUT(float, ColorG) {}
    DEFINE_INPUT(float, ColorB) {}

    DEFINE_OUTPUT_WITH_UPDATE(CSurfaceInfo*, Surface) { UpdateEntity(); }

public:
    DECLARE_META_CLASS(CSurface, CEntity);

    CSurface() = default;

    explicit CSurface(std::string name) : CEntity(std::move(name))
    {
    }
    using CEntity::CEntity;
    void UpdateEntity() override;

private:
    CSurfaceInfo info;
};

}
