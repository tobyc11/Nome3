#pragma once
#include "Mesh.h"

namespace Nome::Scene
{

class CDupin : public CMesh
{
    DEFINE_INPUT(float, A) { MarkDirty(); }
    DEFINE_INPUT(float, B) { MarkDirty(); }
    DEFINE_INPUT(float, C) { MarkDirty(); }
    DEFINE_INPUT(float, D) { MarkDirty(); }
    DEFINE_INPUT(float, U) { MarkDirty(); }
    DEFINE_INPUT(float, V) { MarkDirty(); }
    DEFINE_INPUT(float, Crosssec) { MarkDirty(); }


public:
    DECLARE_META_CLASS(CDupin, CMesh);
    CDupin() = default;
    CDupin(const std::string& name)
        : CMesh(std::move(name))
    {
    }

    void UpdateEntity() override;
};

}
