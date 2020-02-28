#include "Entity.h"

namespace Nome::Scene
{

DEFINE_META_OBJECT(CEntity) {}

CEntity::CEntity()
{
    if (Name.empty())
    {
        static unsigned int nameCounter = 1;
        Name = "entity" + std::to_string(nameCounter++);
    }
}

std::string CEntity::GetNameWithoutPrefix() const
{

    const auto& fullName = GetName();
    size_t lastDot = fullName.find_last_of('.');
    if (lastDot == std::string::npos)
        return fullName;
    return fullName.substr(lastDot + 1);
}

}
