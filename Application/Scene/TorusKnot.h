#pragma once
#include "Mesh.h"
#include "SweepPath.h" // Randy added this. Not sure if correct, check with Zachary

namespace Nome::Scene
{

class CTorusKnot
    : public CSweepPath // Changed to inheret from CSweepPath instead of CMesh. Check with Zachary
{
    DEFINE_INPUT(float, P_Val) { MarkDirty(); }
    DEFINE_INPUT(float, Q_Val) { MarkDirty(); }
    DEFINE_INPUT(float, MajorRadius) { MarkDirty(); }
    DEFINE_INPUT(float, MinorRadius) { MarkDirty(); }
    DEFINE_INPUT(float, TubeRadius) { MarkDirty(); }
    DEFINE_INPUT(float, VerticesPerRing) { MarkDirty(); }
    DEFINE_INPUT(float, Segments) { MarkDirty(); }
    DEFINE_OUTPUT_WITH_UPDATE(CSweepPathInfo*, TorusKnot)
    {
        UpdateEntity();
    } // Randy added this. Check with Zachary

public:
    DECLARE_META_CLASS(CTorusKnot, CMesh);
    CTorusKnot() = default;
    CTorusKnot(const std::string& name)
        : CSweepPath(std::move(name))
    {
    }

    void UpdateEntity() override;
    void MarkDirty() override; // Check with Zachary
};

}