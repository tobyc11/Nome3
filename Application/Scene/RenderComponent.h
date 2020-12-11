#pragma once
#include "Component.h"
#include "Mesh.h"
#include "SceneGraph.h"
#include <Buffer.h>
#include <DrawList.h>

namespace Nome::Scene
{

class CRenderComponent : public CComponent
{
public:
    bool OnAttach() override;
    void OnBeforeDetach() override;
    virtual void Update() = 0;
    virtual void Draw(CDrawList& drawList) = 0;

protected:
    CMeshInstance* MeshInst = nullptr;
    CSceneTreeNode* SceneTreeNode = nullptr;
    std::shared_ptr<CBufferGeometry> Geometry;
    std::shared_ptr<CBufferGeometry> LinesGeometry;
    uint32_t VertexCount, LinesVertexCount;
};

class CMeshRenderComponent : public CRenderComponent
{
public:
    void Update() override;
    void Draw(CDrawList& drawList) override;

private:
    // Controls whether the displayed normal is per-face or per-vertex
    bool bSmoothShading = false;
};

}
