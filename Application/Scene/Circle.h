#pragma once
#include "Mesh.h"

namespace Nome::Scene
{

class CCircle : public CMesh
{
    DEFINE_INPUT(float, Segments) { MarkDirty(); }
    DEFINE_INPUT(float, Radius) { MarkDirty(); }

public:
    DECLARE_META_CLASS(CCircle, CMesh);
    CCircle() = default;
    CCircle(const std::string& name)
        : CMesh(std::move(name))
    {
    }

    void UpdateEntity() override;
};

}
