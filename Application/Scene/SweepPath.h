#pragma once
#include "Mesh.h"

namespace Nome::Scene
{

struct CSweepPathInfo
{
    std::vector<CVertexInfo *> Positions;
    bool IsClosed;
    std::string Name;
};

class CSweepPath : public CMesh
{

public:
    DECLARE_META_CLASS(CSweepPath, CMesh);

    CSweepPath() = default;
    explicit CSweepPath(std::string name)
        : CMesh(std::move(name))
    {
    }

    void UpdateEntity() override;

protected:
    CSweepPathInfo SI;
};

}