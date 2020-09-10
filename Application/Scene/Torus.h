#pragma once
#include "Mesh.h"

namespace Nome::Scene
{

class CTorus : public CMesh
{

    DEFINE_INPUT(float, VerticesPerRing) { MarkDirty(); }
    DEFINE_INPUT(float, MajorRadius) { MarkDirty(); }
    DEFINE_INPUT(float, MinorRadius) { MarkDirty(); }
    DEFINE_INPUT(float, ThetaMax) { MarkDirty(); }
    //DEFINE_INPUT(float, PhiMin) { MarkDirty(); }
    DEFINE_INPUT(float, PhiMax) { MarkDirty(); }
    DEFINE_INPUT(float, Segments) { MarkDirty(); }


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
