#include "Funnel.h"
#include <cmath>

#undef M_PI

namespace Nome::Scene
{

DEFINE_META_OBJECT(CFunnel)
{
    BindPositionalArgument(&CFunnel::VerticesPerRing, 1, 0);
    BindPositionalArgument(&CFunnel::Radius, 1, 1);
    BindPositionalArgument(&CFunnel::Ratio, 1, 2);
    BindPositionalArgument(&CFunnel::Height, 1, 3);
}

void CFunnel::UpdateEntity()
{
    if (!IsDirty())
        return;

    // Clear mesh
    Super::UpdateEntity();

    int n = static_cast<int>(VerticesPerRing.GetValue(16.0f));
    float radius = Radius.GetValue(1.0f);
    float ratio = Ratio.GetValue(0.0f);
    float height = Height.GetValue(1.0f);
    float ri = radius * (1 + ratio);
    for (int i = 0; i < n; i++)
    {
        float theta = (float)i / n * 2.f * (float)tc::M_PI;
        AddVertex("v2_" + std::to_string(i), { radius * cosf(theta), radius * sinf(theta), 0.f });
    }
    for (int i = 0; i < n; i++)
    {
        float theta = (float)i / n * 2.f * (float)tc::M_PI;
        AddVertex("v1_" + std::to_string(i), { ri * cosf(theta), ri * sinf(theta), height });
    }

    // Create faces
    for (int i = 0; i < n; i++)
    {
        // CCW winding
        // v1_next v1_i
        // v2_next v2_i
        int next = (i + 1) % n;
        std::vector<std::string> upperFace = { "v1_" + std::to_string(next),
                                               "v1_" + std::to_string(i), "v2_" + std::to_string(i),
                                               "v2_" + std::to_string(next) };
        AddFace("f1_" + std::to_string(i), upperFace);
    }
    // Two caps
    // std::vector<std::string> upperCap, lowerCap;
    // for (int i = 0; i < n; i++)
    //{
    //    upperCap.push_back("v1_" + std::to_string(i));
    //    lowerCap.push_back("v2_" + std::to_string(n - 1 - i));
    //}
    // AddFace("top", upperCap);
    // AddFace("bottom", lowerCap);
}

}
