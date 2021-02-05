#include "SweepControlPoint.h"

namespace Nome::Scene
{

DEFINE_META_OBJECT(CSweepControlPoint)
{
    Super::CMetaClass();

    BindNamedArgument(&CSweepControlPoint::ScaleX, "scale", 0, 0);
    BindNamedArgument(&CSweepControlPoint::ScaleY, "scale", 0, 1);
    BindNamedArgument(&CSweepControlPoint::ScaleZ, "scale", 0, 2);
    BindNamedArgument(&CSweepControlPoint::RotateX, "rotate", 0, 0);
    BindNamedArgument(&CSweepControlPoint::RotateY, "rotate", 0, 1);
    BindNamedArgument(&CSweepControlPoint::RotateZ, "rotate", 0, 2);
    BindNamedArgument(&CSweepControlPoint::Position, "point", 0);
    BindNamedArgument(&CSweepControlPoint::CrossSection, "cross", 0);
    BindNamedArgument(&CSweepControlPoint::bReverse, "reverse", 0);
}

void CSweepControlPoint::MarkDirty()
{
    Super::MarkDirty();
    SweepControlPoint.MarkDirty();
}

void CSweepControlPoint::UpdateEntity()
{
    Super::UpdateEntity();

    Vector3 Scale = Vector3();
    Scale.x = ScaleX.GetValue(1.0f);
    Scale.y = ScaleY.GetValue(1.0f);
    Scale.z = ScaleZ.GetValue(1.0f);
    SI.Scale = Scale;

    Vector3 Rotate = Vector3();
    Rotate.x = RotateX.GetValue(0.0f);
    Rotate.y = RotateY.GetValue(0.0f);
    Rotate.z = RotateZ.GetValue(0.0f);
    SI.Rotate = Rotate;

    CVertexInfo *info = Position.GetValue(nullptr);
    if (info == nullptr) { return; }
    SI.Position = info->Position;
    SI.CrossSection = CrossSection.GetValue(nullptr);
    SI.Reverse = bReverse;
    SI.Name = GetName();
    SweepControlPoint.UpdateValue(dynamic_cast<CVertexInfo*>(&SI));
    SetValid(true);
}

}
