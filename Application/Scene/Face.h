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
    DECLARE_META_CLASS(CFace, CEntity);
    CFace() = default;
    explicit CFace(std::string name)
        : CEntity(std::move(name))
    {
    }

    void MarkDirty() override;
    void UpdateEntity() override;

    size_t CountVertices() const;
    void SetPointSourceNames(const TAutoPtr<CScene>& scene, std::vector<std::string> points);

    // Returns whether successful
    bool AddFaceIntoMesh(CMesh* mesh) const;

    // AST Handling
    AST::ACommand* MakeCommandNode(AST::CASTContext& ctx, AST::ACommand* parent);
};

}