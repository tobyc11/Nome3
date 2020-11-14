#pragma once
#include "Mesh.h"

namespace Nome::Scene
{

class CHelix : public CMesh
{
    DEFINE_INPUT(float, Radius) { MarkDirty(); }
    DEFINE_INPUT(float, VerticalSpacing) { MarkDirty(); }
    DEFINE_INPUT(float, MaxTheta) { MarkDirty(); }
    DEFINE_INPUT(float, Segments) { MarkDirty(); }

public:
    DECLARE_META_CLASS(CHelix, CMesh);
    CHelix() = default;
    CHelix(const std::string& name)
        : CMesh(std::move(name))
    {
    }

    void UpdateEntity() override;
};

}
