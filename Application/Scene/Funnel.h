#pragma once
#include "Mesh.h"

namespace Nome::Scene
{

class CFunnel : public CMesh
{
    DEFINE_INPUT(float, VerticesPerRing) { MarkDirty(); }
    DEFINE_INPUT(float, Radius) { MarkDirty(); }
    DEFINE_INPUT(float, Height) { MarkDirty(); }
    DEFINE_INPUT(float, Ratio) { MarkDirty(); }

public:
    DECLARE_META_CLASS(CFunnel, CMesh);
    CFunnel() = default;
    CFunnel(const std::string& name)
        : CMesh(std::move(name))
    {
    }

    void UpdateEntity() override;
};

}
