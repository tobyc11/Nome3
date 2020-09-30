#pragma once
#include "Entity.h"

namespace Nome::Scene
{

class CSurface : public CEntity
{
    DEFINE_INPUT(float, ColorR) { }
    DEFINE_INPUT(float, ColorG) { }
    DEFINE_INPUT(float, ColorB) { }

public:
    DECLARE_META_CLASS(CSurface, CEntity);

    using CEntity::CEntity;
};

}