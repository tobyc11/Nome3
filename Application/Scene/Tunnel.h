#pragma once
#include "Mesh.h"

namespace Nome::Scene
{

class CTunnel : public CMesh
{
    DEFINE_INPUT(float, VerticesPerRing) { MarkDirty(); }
    DEFINE_INPUT(float, Radius) { MarkDirty(); }
    DEFINE_INPUT(float, Height) { MarkDirty(); }
    DEFINE_INPUT(float, Ratio) { MarkDirty(); }

public:
    using Super = CMesh;
    CTunnel() = default;
    CTunnel(const std::string& name)
        : CMesh(std::move(name))
    {
    }

    void UpdateEntity() override;
};

}
