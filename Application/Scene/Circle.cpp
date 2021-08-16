#include "Circle.h"

#undef M_PI

namespace Nome::Scene
{

DEFINE_META_OBJECT(CCircle)
{
    BindPositionalArgument(&CCircle::Radius, 1, 0);
    BindPositionalArgument(&CCircle::Segments, 1, 1);
}

void CCircle::MarkDirty()
{
    // Mark this entity dirty
    Super::MarkDirty();

    // And also mark the Face output dirty
    Circle.MarkDirty();
}

void CCircle::UpdateEntity()
{
    if (!IsDirty())
        return;

    Super::UpdateEntity();
    int n = (int)Segments.GetValue(6.0f);
    float radius = Radius.GetValue(1.0f);

    std::vector<Vertex*> handles;
    std::vector<CVertexInfo *> positions;
    for (int i = 0; i < n; i++)
    {
        float theta = (float)i / n * 2.f * (float)tc::M_PI;
        Vector3 pos = Vector3(radius * cosf(theta), radius * sinf(theta), 0.0f);
        handles.push_back(AddVertex("v" + std::to_string(i), pos));
        CVertexInfo *point = new CVertexInfo();
        point->Position = pos;
        positions.push_back(point);
    }
    positions.push_back(positions[0]);
    handles.push_back(handles[0]);

    AddLineStrip("circle", handles);

    // Sweep path info
    SI.Positions = positions;
    SI.IsClosed = true;
    SI.Name = GetName();
    Circle.UpdateValue(&SI);
    SetValid(true);
}

}
