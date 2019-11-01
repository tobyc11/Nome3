#pragma once
#include "Mesh.h"
#include <Matrix3.h>

namespace Nome::Scene
{

using tc::Matrix3;

class IParametricCurve
{
protected:
    // No deletion through this interface, hence protected.
    ~IParametricCurve() = default;

public:
    virtual Matrix3 FrenetFrameAt(float t) = 0;
    virtual std::vector<float> GetDefaultKnots() = 0;
};

class CBezierCurveMath : public IParametricCurve
{
public:
    Matrix3 FrenetFrameAt(float t) override;
    std::vector<float> GetDefaultKnots() override;
    void DeCasteljauInPlace(float t, std::vector<Vector3>& inputOutput);
    std::vector<Vector3> CalcPositions();

    std::vector<Vector3> ControlPoints;
    int Segments;
};

class CBezierSpline : public CMesh
{
    DEFINE_INPUT(float, Segments) { MarkDirty(); }
    DEFINE_INPUT_ARRAY(CVertexInfo*, ControlPoints) { MarkDirty(); }

    DEFINE_OUTPUT_WITH_UPDATE(IParametricCurve*, Spline)
    {
        UpdateEntity();
        Spline.UpdateValue(&Math);
    }

public:
    using Super = CMesh;
    CBezierSpline() = default;
    CBezierSpline(const std::string& name)
        : CMesh(std::move(name))
    {
    }

    void UpdateEntity() override;

private:
    CBezierCurveMath Math;
};

}
