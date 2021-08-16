#pragma once
#include "SweepPath.h"
#include "Point.h"

namespace Nome::Scene
{

struct CSweepControlPointInfo : public CVertexInfo
{
    Vector3 Scale = Vector3(1.0f, 1.0f, 1.0f);
    Vector3 Rotate;
    bool Reverse;
    CSweepPathInfo *CrossSection;
};

class CSweepControlPoint : public CEntity {
    DEFINE_INPUT(float, ScaleX) {
        MarkDirty();
    }
    DEFINE_INPUT(float, ScaleY) { MarkDirty(); }
    DEFINE_INPUT(float, ScaleZ) { MarkDirty(); }
    DEFINE_INPUT(float, RotateX) { MarkDirty(); }
    DEFINE_INPUT(float, RotateY) { MarkDirty(); }
    DEFINE_INPUT(float, RotateZ) { MarkDirty(); }
    DEFINE_INPUT(CVertexInfo *, Position) { MarkDirty(); }
    DEFINE_INPUT(CSweepPathInfo *, CrossSection) { MarkDirty(); }

    DEFINE_OUTPUT_WITH_UPDATE(CVertexInfo*, SweepControlPoint) { UpdateEntity(); }

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
    CSweepControlPointInfo SI;
    bool bReverse = false;
};

}
