#pragma once
#include "Mesh.h"

namespace Nome::Scene
{

class CTorusKnot : public CMesh
{
    DEFINE_INPUT(float, P_Val) { MarkDirty(); }
    DEFINE_INPUT(float, Q_Val) { MarkDirty(); }
    DEFINE_INPUT(float, MajorRadius) { MarkDirty(); }
    DEFINE_INPUT(float, MinorRadius) { MarkDirty(); }
    DEFINE_INPUT(float, TubeRadius) { MarkDirty(); }
    DEFINE_INPUT(float, VerticesPerRing) { MarkDirty(); }
    DEFINE_INPUT(float, Segments) { MarkDirty(); }


public:

    DECLARE_META_CLASS(CTorusKnot, CMesh);
    CTorusKnot() = default;
    CTorusKnot(const std::string& name)
        : CMesh(std::move(name))
    {
    }

    void UpdateEntity() override;
};

}
