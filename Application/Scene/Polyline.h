#pragma once
#include "Mesh.h"

namespace Nome::Scene
{

class CPolyline : public CMesh
{
    DEFINE_INPUT_ARRAY(CVertexInfo*, Points) { MarkDirty(); }

public:
    DECLARE_META_CLASS(CPolyline, CMesh);

    CPolyline() = default;
    explicit CPolyline(std::string name)
        : CMesh(std::move(name))
    {
    }

    void UpdateEntity() override;

    void SetClosed(bool closed);

private:
    bool bClosed = false;
};

}
