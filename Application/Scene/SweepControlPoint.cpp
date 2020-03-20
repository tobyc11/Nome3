#include "SweepControlPoint.h"

namespace Nome::Scene
{

DEFINE_META_OBJECT(CSweepControlPoint)
{
    BindPositionalArgument(&CSweepControlPoint::OwnerName, 1, 0);
    BindPositionalArgument(&CSweepControlPoint::ScaleX, 1, 1);
    BindPositionalArgument(&CSweepControlPoint::ScaleY, 1, 2);
    BindPositionalArgument(&CSweepControlPoint::Rotate, 1, 3);
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
    SI.OwnerName = OwnerName;
    SI.Name = GetName();
    SweepControlPoint.UpdateValue(&SI);
    SetValid(true);
}

}
