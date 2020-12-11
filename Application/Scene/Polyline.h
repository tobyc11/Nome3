#pragma once
#include "Mesh.h"
#include "SweepPath.h"

namespace Nome::Scene
{

class CPolyline : public CSweepPath
{
    DEFINE_INPUT_ARRAY(CVertexInfo*, Points) { MarkDirty(); }

    DEFINE_OUTPUT_WITH_UPDATE(CSweepPathInfo*, Polyline) { UpdateEntity(); }

public:
    DECLARE_META_CLASS(CPolyline, CSweepPath);

    CPolyline() = default;
    explicit CPolyline(std::string name)
        : CSweepPath(std::move(name))
    {
    }

    void MarkDirty() override;
    void UpdateEntity() override;
    void SetPointSourceNames(const TAutoPtr<CScene>& scene, std::vector<std::string> points);
    void SetClosed(bool closed);
    AST::ACommand* SyncToAST(AST::CASTContext& ctx, bool createNewNode) override; // Randy added this

private:
    bool bClosed = false;
    std::vector<std::string> PointSource;
};

}
