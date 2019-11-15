#pragma once
#include "Point.h"
#include <Flow/FlowNodeArray.h>

namespace Nome::Scene
{

class CScene;
class CMesh;

class CFace : public CEntity
{
    // Face points, in order
    DEFINE_INPUT_ARRAY(CVertexInfo*, Points) { MarkDirty(); }

    DEFINE_OUTPUT_WITH_UPDATE(CFace*, Face) { UpdateEntity(); }

public:
    using Super = CEntity;
    CFace() = default;
    explicit CFace(std::string name)
        : CEntity(std::move(name))
    {
    }

    void MarkDirty() override;
    void UpdateEntity() override;

    size_t CountVertices() const;
    std::vector<std::string> GetPointSourceNames() const;
    void SetPointSourceNames(const TAutoPtr<CScene>& scene, std::vector<std::string> points);

    // Returns whether successful
    bool AddFaceIntoMesh(CMesh* mesh) const;

private:
    // TODO: this should be implemented as AST binding
    std::vector<std::string> PointSource;
};

}
