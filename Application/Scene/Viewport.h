#pragma once
#include <QRect>
#include "Entity.h"

namespace Nome::Scene
{

class CViewport : public CEntity
{
DEFINE_INPUT(float, rect_x1) { MarkDirty(); }
DEFINE_INPUT(float, rect_x2) { MarkDirty(); }
DEFINE_INPUT(float, rect_y1) { MarkDirty(); }
DEFINE_INPUT(float, rect_y2) { MarkDirty(); }

public:
    DECLARE_META_CLASS(CViewport, CEntity);

    CViewport() = default;
    explicit CViewport(const std::string& name)
        : CEntity(std::move(name))
    {
        renderType = VIEWPORT;
    }



    void UpdateEntity() override;
    void MarkDirty() override;
    bool IsInstantiable() override;
    bool IsMesh() override;
    CEntity* Instantiate(CSceneTreeNode* treeNode) override;
    QRectF viewport;
    std::string cameraId;

private:
    CSceneTreeNode* SceneTreeNode;
};

}
