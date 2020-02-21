#pragma once
#include "Mesh.h"
#include "Vector3.h"

namespace Nome::Scene
{

class CPolyline : public CMesh
{
    DEFINE_INPUT_ARRAY(CVertexInfo*, Points) { MarkDirty(); }

public:
    using Super = CMesh;
    CPolyline() = default;
    CPolyline(const std::string& name)
        : CMesh(std::move(name))
    {
    }

    void UpdateEntity() override;

    void SetClosed(bool closed);

private:
    void drawCircle(Vector3 center, Vector3 N, Vector3 B, float radius, float angles, float scale, int num_phi, int index);
    bool bClosed = false;
};

}
