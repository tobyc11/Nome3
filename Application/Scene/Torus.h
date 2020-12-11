#pragma once
#include "Mesh.h"

namespace Nome::Scene
{

class CTorus : public CMesh
{

    DEFINE_INPUT(float, maj_rad) { MarkDirty(); }
    DEFINE_INPUT(float, min_rad) { MarkDirty(); }
    DEFINE_INPUT(float, theta_max) { MarkDirty(); }
    DEFINE_INPUT(float, phi_min) { MarkDirty(); }
    DEFINE_INPUT(float, phi_max) { MarkDirty(); }
    DEFINE_INPUT(float, theta_segs) { MarkDirty(); }
    DEFINE_INPUT(float, phi_segs) { MarkDirty(); }

public:
    DECLARE_META_CLASS(CTorus, CMesh);
    CTorus() = default;
    CTorus(const std::string& name)
        : CMesh(std::move(name))
    {
    }

    void UpdateEntity() override;
};

}