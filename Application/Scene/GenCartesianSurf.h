#pragma once
#include "Mesh.h"

namespace Nome::Scene
{

class CGenCartesianSurf : public CMesh
{
    DEFINE_INPUT(float, x_start) { MarkDirty(); }
    DEFINE_INPUT(float, x_end) { MarkDirty(); }
    DEFINE_INPUT(float, y_start) { MarkDirty(); }
    DEFINE_INPUT(float, y_end) { MarkDirty(); }
    DEFINE_INPUT(float, x_segs) { MarkDirty(); }
    DEFINE_INPUT(float, y_segs) { MarkDirty(); }

public:
    DECLARE_META_CLASS(CGenCartesianSurf, CMesh);
    CGenCartesianSurf() = default;
    CGenCartesianSurf(const std::string& name)
        : CMesh(std::move(name))
    {
    }

    void UpdateEntity() override;


private:
    //..
};

}
