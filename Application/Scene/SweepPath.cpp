#include "SweepPath.h"

namespace Nome::Scene
{

DEFINE_META_OBJECT(CSweepPath)
{
}

void CSweepPath::UpdateEntity()
{
    if (!IsDirty())
        return;
    Super::UpdateEntity();
}

}
