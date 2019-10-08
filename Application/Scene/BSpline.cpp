#include "BSpline.h"

namespace Nome::Scene
{

Matrix3 CBSplineMath::FrenetFrameAt(float t)
{
    //TODO: starter code
    return Matrix3::IDENTITY;
}

std::vector<float> CBSplineMath::GetDefaultKnots()
{
    //TODO: starter code
    std::vector<float> result = { 0.0f, 0.5f, 1.0f };
    return result;
}

void CBSpline::SetClosed(bool closed)
{
    bClosed = closed;
}

void CBSpline::UpdateEntity()
{
    if (!IsDirty())
        return;

    Super::UpdateEntity();

    int n = (int) Segments.GetValue(8.0f);

    //TODO: starter code
    for (int i = 0; i <= n; i++)
    {
        //Sample some dummy values
        SamplePositions.emplace_back(i * 1.0f, 1.0f, 0.0f);
    }

    std::vector<CMeshImpl::VertexHandle> handles;
    handles.reserve(n + 1);
    for (int i = 0; i < n + 1; i++)
    {
        handles.push_back(AddVertex("v" + std::to_string(i), SamplePositions[i]));
    }
    AddLineStrip("curve", handles);
}

void CBSpline::Draw(IDebugDraw* draw)
{
    tc::Color c1 = tc::Color::YELLOW;
    tc::Color c2 = tc::Color::RED;
    auto iter = SamplePositions.begin();
    while (iter != SamplePositions.end() - 1)
    {
        const auto& p0 = *iter;
        const auto& p1 = *(++iter);
        draw->LineSegment(p0, c1, p1, c2);
        std::swap(c1, c2);
    }
}

}
