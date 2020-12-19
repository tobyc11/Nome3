#pragma once
#include "Mesh.h"

namespace Nome::Scene
{

class CEllipsoid : public CMesh
{
    DEFINE_INPUT(float, RadiusX) { MarkDirty(); }
    DEFINE_INPUT(float, RadiusY) { MarkDirty(); }
    DEFINE_INPUT(float, ThetaMax) { MarkDirty(); }
    DEFINE_INPUT(float, PhiMin) { MarkDirty(); }
    DEFINE_INPUT(float, PhiMax) { MarkDirty(); }
    DEFINE_INPUT(float, ThetaSegs) { MarkDirty(); }
    DEFINE_INPUT(float, PhiSegs) { MarkDirty(); }

public:
    DECLARE_META_CLASS(CEllipsoid, CMesh);
    CEllipsoid() = default;
    CEllipsoid(const std::string& name)
        : CMesh(std::move(name))
    {
    }

    void UpdateEntity() override;
};

}
