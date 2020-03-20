#pragma once
#include "Entity.h"

namespace Nome::Scene
{

struct CSweepControlPointInfo
{
    float ScaleX;
    float ScaleY;
    float Rotate;
    std::string OwnerName;
    std::string Name;
};

class CSweepControlPoint : public CEntity {
    DEFINE_INPUT(float, ScaleX) { MarkDirty(); }
    DEFINE_INPUT(float, ScaleY) { MarkDirty(); }
    DEFINE_INPUT(float, Rotate) { MarkDirty(); }

DEFINE_OUTPUT_WITH_UPDATE(CSweepControlPointInfo*, SweepControlPoint) { UpdateEntity(); }

    void MarkDirty() override;
    void UpdateEntity() override;

public:
    DECLARE_META_CLASS(CSweepControlPoint, CEntity);

    CSweepControlPoint() = default;

    explicit CSweepControlPoint(std::string name)
            : CEntity(std::move(name))
    {
    }

private:
    std::string OwnerName;
    CSweepControlPointInfo SI;
};

}
