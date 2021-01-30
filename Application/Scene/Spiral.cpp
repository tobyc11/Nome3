#include "Spiral.h"

#undef M_PI

namespace Nome::Scene
{

DEFINE_META_OBJECT(SSpiral)
{
    BindPositionalArgument(&SSpiral::Rate, 1, 0);
    BindPositionalArgument(&SSpiral::Angle, 1, 1);
    BindPositionalArgument(&SSpiral::Segments, 1, 2);
}

void SSpiral::UpdateEntity()
{
    if (!IsDirty())
        return;

    Super::UpdateEntity();
    float r = Rate.GetValue(6.0f);
    int n = (int)Segments.GetValue(6.0f);
    int ang = (int)Angle.GetValue(6.0f);

    float radius = 0;

    std::vector<CMeshImpl::VertexHandle> handles;
    for (int i = 0; i < n; i++)
    {
        float theta = (float)i / n * 2.f * (float)ang;
        AddVertex("u" + std::to_string(i), {radius * cosf(theta), radius * sinf(theta), 0.0f});
        AddVertex("l" + std::to_string(i), {radius * cosf(theta), radius * sinf(theta), -0.13});

        radius = radius + r;
    }
    for (int i = 0; i < n - 1; i++) {
        std::vector<std::string> face1 = {"u" + std::to_string(i + 1), "u" + std::to_string(i), "l" + std::to_string(i)};
        AddFace("f1_" + std::to_string(i), face1);
        std::vector<std::string> face2 = {"l" + std::to_string(i), "l" + std::to_string(i + 1), "u" + std::to_string(i + 1)};
        AddFace("f2_" + std::to_string(i), face2);    
    }


}

}
