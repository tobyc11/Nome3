#include "SweepControlPoint.h"

namespace Nome::Scene
{

DEFINE_META_OBJECT(CSweepControlPoint)
{
    Super::CMetaClass();

    BindPositionalArgument(&CSweepControlPoint::ScaleX, 1, 1);
    BindPositionalArgument(&CSweepControlPoint::ScaleY, 1, 2);
    BindPositionalArgument(&CSweepControlPoint::Rotate, 1, 3);
    BindPositionalArgument(&CSweepControlPoint::Range, 1, 4);
}

void CSweepControlPoint::MarkDirty()
{
    Super::MarkDirty();
    SweepControlPoint.MarkDirty();
}

void CSweepControlPoint::UpdateEntity()
{
    Super::UpdateEntity();
    SI.ScaleX = ScaleX.GetValue(1.0f);
    SI.ScaleY = ScaleY.GetValue(1.0f);
    SI.Rotate = Rotate.GetValue(0.0f);
    SI.Range = Range.GetValue(0.0f);
    SI.OwnerName = OwnerName;
    SI.Name = GetName();
    SweepControlPoint.UpdateValue(dynamic_cast<CControlPointInfo*>(&SI));
    SetValid(true);
}

}
