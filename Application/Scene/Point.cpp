#include "Point.h"

namespace Nome::Scene
{

DEFINE_META_OBJECT(CPoint)
{
    BindPositionalArgument(&CPoint::X, 1, 0);
    BindPositionalArgument(&CPoint::Y, 1, 1);
    BindPositionalArgument(&CPoint::Z, 1, 2);
}

void CPoint::MarkDirty()
{
    Super::MarkDirty();
    Point.MarkDirty();
}

void CPoint::UpdateEntity()
{
    Super::UpdateEntity();


    VI.Position = { X.GetValue(0.0f), Y.GetValue(0.0f), Z.GetValue(0.0f) };
    VI.Name = GetName();
    Point.UpdateValue(&VI);
    SetValid(true);
}

}
