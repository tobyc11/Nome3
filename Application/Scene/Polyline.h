#pragma once
#include "Mesh.h"
#include "Point.h"
#include <Flow/FlowNodeArray.h>

namespace Nome::Scene
{

class CPolyline : public CMesh
{
    DEFINE_INPUT_ARRAY(CVertexInfo*, Points)
    {
        MarkDirty();
    }

public:
    using Super = CMesh;
    CPolyline() = default;
    CPolyline(const std::string& name) : CMesh(std::move(name)) {}

    void UpdateEntity() override;
};

}
