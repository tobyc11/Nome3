#pragma once
#include "BezierSpline.h"

namespace Nome::Scene
{

class CBSplineMath : public IParametricCurve
{
public:
    ///Get the frenet frame at t
    Matrix3 FrenetFrameAt(float t) override;

    ///Get a default list of t values to use
    std::vector<float> GetDefaultKnots() override;
};

class CBSpline : public CMesh
{
    DEFINE_INPUT(float, Order) { MarkDirty(); }
    DEFINE_INPUT(float, Segments) { MarkDirty(); }
    DEFINE_INPUT_ARRAY(CVertexInfo*, ControlPoints) { MarkDirty(); }

    DEFINE_OUTPUT_WITH_UPDATE(IParametricCurve*, Spline) { UpdateEntity(); Spline.UpdateValue(&Math); }

public:
    using Super = CMesh;
    CBSpline() = default;
    CBSpline(const std::string& name) : CMesh(name) {}

    ///Set whether the bspline is a closed loop
    void SetClosed(bool closed);

    void UpdateEntity() override;
    void Draw(IDebugDraw* draw) override;

private:
    CBSplineMath Math;
    bool bClosed = false;
    std::vector<Vector3> SamplePositions;
};

}
