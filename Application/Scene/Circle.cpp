#include "Circle.h"

namespace Nome::Scene
{

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
        handles.push_back(AddVertex("v" + std::to_string(i),
                                    { radius * cosf(theta), radius * sinf(theta), 0.0f }));
    }
    handles.push_back(handles[0]);
    AddLineStrip("circle", handles);
}

}
