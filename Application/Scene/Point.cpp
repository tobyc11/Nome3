#include "Point.h"

namespace Nome::Scene
{

DEFINE_META_OBJECT(CPoint)
{
    BindPositionalArgument(&CPoint::X, 1, 0);
    BindPositionalArgument(&CPoint::Y, 1, 1);
    BindPositionalArgument(&CPoint::Z, 1, 2);
    BindPositionalArgument(&CPoint::ControlPoints, 2);
}

void CPoint::MarkDirty()
{
    Super::MarkDirty();
    Point.MarkDirty();
}

void CPoint::UpdateEntity()
{
    Super::UpdateEntity();

    std::vector<CControlPointInfo*> controlPoints;
    for (size_t i = 0; i < ControlPoints.GetSize(); i++)
    {
        CControlPointInfo* CI = ControlPoints.GetValue(i, nullptr);
        if (CI != NULL) { controlPoints.push_back(CI); }
    }

    VI.Position = { X.GetValue(0.0f), Y.GetValue(0.0f), Z.GetValue(0.0f) };
    VI.Name = GetName();
    VI.ControlPoints = controlPoints;
    Point.UpdateValue(&VI);
    SetValid(true);
}

}
