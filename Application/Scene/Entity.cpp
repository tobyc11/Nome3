#include "Entity.h"

namespace Nome::Scene
{

std::string CEntity::GenerateSequentialName()
{
    static unsigned int nameCounter = 1;
    return "entity" + std::to_string(nameCounter++);
}

void CEntity::MarkDirty()
{
    bIsDirty = true;
    //TODO: notify the scene
}

}
