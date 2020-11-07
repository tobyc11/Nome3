#pragma once
#include "Mesh.h"

namespace Nome::Scene
{

class CMobiusStrip : public CMesh
{
    DEFINE_INPUT(float, N) { MarkDirty(); }
    DEFINE_INPUT(float, Radius) { MarkDirty(); }
    DEFINE_INPUT(float, NumTwists) { MarkDirty(); }
    DEFINE_INPUT(float, NumCuts) { MarkDirty(); }

public:
    DECLARE_META_CLASS(CMobiusStrip, CMesh);
    CMobiusStrip() = default;
    CMobiusStrip(const std::string& name)
        : CMesh(std::move(name))
    {
    }

    void UpdateEntity() override;

private:
    // ...
};

}
