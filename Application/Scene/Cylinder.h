#pragma once
#include "Mesh.h"

namespace Nome::Scene
{

class CCylinder : public CMesh
{
    DEFINE_INPUT(float, Radius) { MarkDirty(); }
    DEFINE_INPUT(float, Height) { MarkDirty(); }
    DEFINE_INPUT(float, ThetaMax) { MarkDirty(); }
    DEFINE_INPUT(float, ThetaSegs) { MarkDirty(); }

public:
    DECLARE_META_CLASS(CCylinder, CMesh);
    CCylinder() = default;
    CCylinder(const std::string& name)
        : CMesh(std::move(name))
    {
    }

    void UpdateEntity() override;

private:
    bool bBotCap = false;
    bool bTopCap = false;
};

}
