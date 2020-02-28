#pragma once
#include "BezierSpline.h"

namespace Nome::Scene
{

class CBSplineMath : public IParametricCurve
{
public:
    /// Get the frenet frame at t
    Matrix3 FrenetFrameAt(float t) override;

    /// Get a default list of t values to use
    std::vector<float> GetDefaultKnots() override;
};

class CBSpline : public CMesh
{
    DEFINE_INPUT(float, Order) { MarkDirty(); }
    DEFINE_INPUT(float, Segments) { MarkDirty(); }
    DEFINE_INPUT_ARRAY(CVertexInfo*, ControlPoints) { MarkDirty(); }

    DEFINE_OUTPUT_WITH_UPDATE(IParametricCurve*, Spline)
    {
        UpdateEntity();
        Spline.UpdateValue(&Math);
    }

public:
    DECLARE_META_CLASS(CBSpline, CMesh);

    CBSpline() = default;
    explicit CBSpline(const std::string& name)
        : CMesh(name)
    {
    }

    /// Set whether the bspline is a closed loop
    void SetClosed(bool closed);
    std::vector<float> GetDefKnots();
    int GetKnotInt(float x, std::vector<float> knots);
    Vector3 deBoor(int k , int degree, int i, double x, std::vector<float> knots);
    void UpdateEntity() override;
    void Draw(IDebugDraw* draw) override;
    Vector3 deBoor1(int k,  double x, std::vector<float> knots, int p);
    int NFactor(int i, int j, float t);
    Vector3 FindPoint(int degree, float t);

private:
    CBSplineMath Math;
    bool bClosed = false;
    std::vector<Vector3> SamplePositions;
};

}
