#include "Entity.h"

namespace Nome::Scene
{

void CEntity::MarkDirty()
{
    bIsDirty = true;
    //TODO: notify the scene
}

}
