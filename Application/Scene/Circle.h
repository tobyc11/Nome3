#pragma once
#include "Mesh.h"
#include "SweepPath.h"

namespace Nome::Scene
{

class CCircle : public CSweepPath
{
    DEFINE_INPUT(float, Segments) { MarkDirty(); }
    DEFINE_INPUT(float, Radius) { MarkDirty(); }
    DEFINE_OUTPUT_WITH_UPDATE(CSweepPathInfo*, Circle) { UpdateEntity(); }

public:
    DECLARE_META_CLASS(CCircle, CSweepPath);
    CCircle() = default;
    CCircle(const std::string& name)
        : CSweepPath(std::move(name))
    {
    }

    void UpdateEntity() override;

private:
    std::vector<CVertexInfo> points;
};

}
