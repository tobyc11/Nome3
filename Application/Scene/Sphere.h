#pragma once
#include "Mesh.h"

namespace Nome::Scene
{

class CSphere : public CMesh
{
    DEFINE_INPUT(float, Radius) { MarkDirty(); }
    DEFINE_INPUT(float, ThetaMax) { MarkDirty(); }
    DEFINE_INPUT(float, PhiMin) { MarkDirty(); }
    DEFINE_INPUT(float, PhiMax) { MarkDirty(); }
    DEFINE_INPUT(float, ThetaSegs) { MarkDirty(); }
    DEFINE_INPUT(float, PhiSegs) { MarkDirty(); }

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
