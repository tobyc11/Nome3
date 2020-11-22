#include "Helix.h"

#undef M_PI

namespace Nome::Scene
{

DEFINE_META_OBJECT(CHelix)
{
    BindPositionalArgument(&CHelix::Radius, 1, 0);
    BindPositionalArgument(&CHelix::VerticalSpacing, 1, 1);
    BindPositionalArgument(&CHelix::MaxTheta, 1, 2);
    BindPositionalArgument(&CHelix::Segments, 1, 3);
}

void CHelix::UpdateEntity()
{
    if (!IsDirty())
        return;

    Super::UpdateEntity();
    int n = (int)Segments.GetValue(6.0f);
    float radius = Radius.GetValue(1.0f);
    float c = VerticalSpacing.GetValue(1.0f);
    float max_theta = MaxTheta.GetValue(1.0f);

    std::vector<CMeshImpl::VertexHandle> handles;
    for (int i = 0; i < n; i++)
    {
        float theta = (float)i / n * (max_theta / 360.f) * 2.f * (float)tc::M_PI;
        handles.push_back(AddVertex("v" + std::to_string(i),
                                    { radius * cosf(theta), radius * sinf(theta), c * theta }));
    }
    AddLineStrip("helix", handles);
}

}
