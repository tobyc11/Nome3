#pragma once
#include "Entity.h"

namespace Nome::Scene
{

struct CControlPointInfo
{
    std::string OwnerName;
    std::string Name;

    virtual ~CControlPointInfo() = default;
};

class CControlPoint : public CEntity
{
public:
    DECLARE_META_CLASS(CControlPoint, CEntity);

    CControlPoint() = default;

    explicit CControlPoint(std::string name)
            : CEntity(std::move(name))
    {
    }

protected:
    std::string OwnerName;
};

}
