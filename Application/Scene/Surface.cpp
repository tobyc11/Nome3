#include "Surface.h"

namespace Nome::Scene
{

DEFINE_META_OBJECT(CSurface)
{
    BindNamedArgument(&CSurface::ColorR, "color", 0, 0);
    BindNamedArgument(&CSurface::ColorG, "color", 0, 1);
    BindNamedArgument(&CSurface::ColorB, "color", 0, 2);
}

void CSurface::UpdateEntity() {

    Super::UpdateEntity();

    info.surfacename = GetName();
    info.ColorR = ColorR.GetValue(0.0f);
    info.ColorG = ColorG.GetValue(0.0f);
    info.ColorB = ColorB.GetValue(0.0f);
}

}
