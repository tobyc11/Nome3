#include "Circle.h"

#undef M_PI

namespace Nome::Scene
{

DEFINE_META_OBJECT(CCircle)
{
    BindPositionalArgument(&CCircle::Segments, 1, 0);
    BindPositionalArgument(&CCircle::Radius, 1, 1);
}

void CCircle::UpdateEntity()
{
    if (!IsDirty())
        return;

    Super::UpdateEntity();
    int n = (int)Segments.GetValue(6.0f);
    float radius = Radius.GetValue(1.0f);

    std::vector<CMeshImpl::VertexHandle> handles;
    for (int i = 0; i < n; i++)
    {
        float theta = (float)i / n * 2.f * (float)tc::M_PI;
        Vector3 pos = Vector3(radius * cosf(theta), radius * sinf(theta), 0.0f);
        handles.push_back(AddVertex("v" + std::to_string(i), pos));
        CVertexInfo point;
        point.Position = pos;
        points.push_back(point);
    }
    points.push_back(points[0]);
    handles.push_back(handles[0]);

    std::vector<CVertexInfo *> positions;
    for (int i = 0; i < n + 1; i++)
        positions.push_back(&points[i]);
    AddLineStrip("circle", handles);

    SI.Positions = positions;
    SI.IsClosed = true;
    SI.Name = GetName();
    Circle.UpdateValue(&SI);
}

}
