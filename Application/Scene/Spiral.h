#pragma once
#include "Mesh.h"

namespace Nome::Scene
{

class SSpiral : public CMesh
{
    DEFINE_INPUT(float, Rate) { MarkDirty(); }
    DEFINE_INPUT(float, Angle) { MarkDirty(); }
    DEFINE_INPUT(float, Segments) { MarkDirty(); }


public:
    DECLARE_META_CLASS(SSpiral, CMesh);
    SSpiral() = default;
    SSpiral(const std::string& name)
        : CMesh(std::move(name))
    {
    }

    void UpdateEntity() override;
};

}