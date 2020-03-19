#pragma once
#include "Mesh.h"
#include <vector>

namespace Nome::Scene
{

struct CPolylineInfo
{
    std::vector<CVertexInfo*> Positions;
    bool IsClosed;
    std::string Name;
};

class CPolyline : public CMesh
{
    DEFINE_INPUT_ARRAY(CVertexInfo*, Points) { MarkDirty(); }

    DEFINE_OUTPUT_WITH_UPDATE(CPolylineInfo*, Polyline) { UpdateEntity(); }

public:
    DECLARE_META_CLASS(CPolyline, CMesh);

    CPolyline() = default;
    explicit CPolyline(std::string name)
        : CMesh(std::move(name))
    {
    }

    void MarkDirty() override;
    void UpdateEntity() override;

    void SetClosed(bool closed);

private:
    bool bClosed = false;

    CPolylineInfo PI;
};

}
