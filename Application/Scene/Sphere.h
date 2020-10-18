#pragma once
#include "Mesh.h"

namespace Nome::Scene
{

class CSphere : public CMesh
{
    DEFINE_INPUT(float, Segments) { MarkDirty(); }
    DEFINE_INPUT(float, Radius) { MarkDirty(); }
    DEFINE_INPUT(float, CrossSections) { MarkDirty(); }
    DEFINE_INPUT(float, MaxTheta) { MarkDirty(); }
    DEFINE_INPUT(float, MinPhi) { MarkDirty(); }
    DEFINE_INPUT(float, MaxPhi) { MarkDirty(); }

public:
    DECLARE_META_CLASS(CSphere, CMesh);
    CSphere() = default;
    CSphere(const std::string& name)
        : CMesh(std::move(name))
    {
    }

    void UpdateEntity() override;
};

}
