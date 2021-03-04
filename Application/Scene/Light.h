#pragma once
#include "Entity.h"
#include <Flow/FlowNodeArray.h>

namespace Nome::Scene
{

struct CLightInfo
{
    Vector3 color;
    std::string type;
    std::string name;

    virtual ~CLightInfo() = default;
};


class CLight : public CEntity
{
DEFINE_INPUT(float, R) { MarkDirty(); }
DEFINE_INPUT(float, G) { MarkDirty(); }
DEFINE_INPUT(float, B) { MarkDirty(); }
DEFINE_INPUT(std::string, type) { MarkDirty(); }



public:
    DECLARE_META_CLASS(CLight, CEntity);
    CLight() = default;
    explicit CLight(std::string name)
        : CEntity(std::move(name))
    {
    }

    void MarkDirty() override;
    void UpdateEntity() override;

private:
    CLightInfo LI;
};

}