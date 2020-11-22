#pragma once
#include "Mesh.h"

namespace Nome::Scene
{

class CHyperboloid : public CMesh
{
    DEFINE_INPUT(float, Segments) { MarkDirty(); }
    DEFINE_INPUT(float, Radius) { MarkDirty(); }
    DEFINE_INPUT(float, B) { MarkDirty(); }
    DEFINE_INPUT(float, C) { MarkDirty(); }
    DEFINE_INPUT(float, Sheet) { MarkDirty(); }
    DEFINE_INPUT(float, Theta) { MarkDirty(); }

public:
    DECLARE_META_CLASS(CHyperboloid, CMesh);
    CHyperboloid() = default;
    CHyperboloid(const std::string& name)
        : CMesh(std::move(name))
    {
    }

    void UpdateEntity() override;
};

}
