#include "BezierSpline.h"

namespace Nome::Scene
{

DEFINE_META_OBJECT(CBezierSpline)
{
    BindPositionalArgument(&CBezierSpline::ControlPoints, 1);
    BindNamedArgument(&CBezierSpline::Segments, "segs", 0);
}

Matrix3 CBezierCurveMath::FrenetFrameAt(float t) { return Matrix3(); }

std::vector<float> CBezierCurveMath::GetDefaultKnots()
{
    std::vector<float> result;
    float step = 1.0f / Segments;
    for (int i = 0; i < Segments; i++)
        result.push_back(i * step);
    result.push_back(1.0f);
    return result;
}

void CBezierCurveMath::DeCasteljauInPlace(float t, std::vector<Vector3>& inputOutput)
{
    size_t sz = inputOutput.size();
    while (sz > 1)
    {
        for (size_t i = 0; i < sz - 1; i++)
        {
            inputOutput[i] = (1 - t) * inputOutput[i] + t * inputOutput[i + 1];
        }
        sz--;
    }
}

std::vector<Vector3> CBezierCurveMath::CalcPositions()
{
    std::vector<Vector3> result;
    auto knots = GetDefaultKnots();
    for (float t : knots)
    {
        std::vector<Vector3> temp = ControlPoints;
        DeCasteljauInPlace(t, temp);
        result.push_back(temp[0]);
    }
    return result;
}

void CBezierSpline::UpdateEntity()
{
    if (!IsDirty())
        return;

    Super::UpdateEntity();

    int n = (int)Segments.GetValue(8.0f);

    Math.ControlPoints.clear();
    size_t cpn = ControlPoints.GetSize();
    for (size_t i = 0; i < cpn; i++)
    {
        Math.ControlPoints.push_back(ControlPoints.GetValue(i, nullptr)->Position);
    }
    Math.Segments = n;
    std::vector<Vector3> positions = Math.CalcPositions();
    assert(positions.size() == n + 1);

    std::vector<Vertex*> handles;
    for (int i = 0; i < n + 1; i++)
    {
        handles.push_back(AddVertex("v" + std::to_string(i), positions[i]));
        CVertexInfo point;
        point.Position = positions[i];
        points.push_back(point);
    }
    std::vector<CVertexInfo *> Positions;
    for (int i = 0; i < n + 1; i++)
        Positions.push_back(&points[i]);
    SI.Positions = Positions;
    SI.Name = GetName();
    BezierSpline.UpdateValue(&SI);

    AddLineStrip("curve", handles);
}

}
