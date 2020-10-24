#pragma once
#include "ControlPoint.h"
#include "SweepPath.h"

namespace Nome::Scene
{

struct CSweepControlPointInfo : public CControlPointInfo
{
    float ScaleX;
    float ScaleY;
    float Rotate;
    float Range;
    CSweepPathInfo *CrossSection;
};

class CSweepControlPoint : public CControlPoint {
    DEFINE_INPUT(float, ScaleX) { MarkDirty(); }
    DEFINE_INPUT(float, ScaleY) { MarkDirty(); }
    DEFINE_INPUT(float, Rotate) { MarkDirty(); }
    DEFINE_INPUT(float, Range) { MarkDirty(); }
    DEFINE_INPUT(CSweepPathInfo *, CrossSection) { MarkDirty(); }

    DEFINE_OUTPUT_WITH_UPDATE(CControlPointInfo*, SweepControlPoint) { UpdateEntity(); }

    void MarkDirty() override;
    void UpdateEntity() override;

public:
    DECLARE_META_CLASS(CSweepControlPoint, CControlPoint);

    CSweepControlPoint() = default;

    explicit CSweepControlPoint(std::string name)
            : CControlPoint(std::move(name))
    {
    }

private:
    CSweepControlPointInfo SI;
};

}
