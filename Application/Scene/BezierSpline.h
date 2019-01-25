#pragma once
#include "Mesh.h"

namespace Nome::Scene
{

//TODO
class CBezierSpline : public CMesh
{
	DEFINE_INPUT(float, Segments) { MarkDirty(); }
	DEFINE_INPUT(float, Radius) { MarkDirty(); }

public:
    using Super = CMesh;
	CBezierSpline() = default;
	CBezierSpline(const std::string& name) : CMesh(std::move(name)) {}

    void UpdateEntity() override;
};

}
