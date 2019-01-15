#include "Entity.h"

namespace Nome::Scene
{

CEntity::CEntity()
{
    if (Name.empty())
    {
        static unsigned int nameCounter = 1;
        Name = "entity" + std::to_string(nameCounter++);
    }
}

}
