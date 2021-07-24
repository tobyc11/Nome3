#pragma once
#include <QRect>
#include "Entity.h"

namespace Nome::Scene
{

class CViewport : public CEntity
{
    DEFINE_INPUT(float, origin_x) { MarkDirty(); }
    DEFINE_INPUT(float, origin_y) { MarkDirty(); }
    DEFINE_INPUT(float, length) { MarkDirty(); }
    DEFINE_INPUT(float, width) { MarkDirty(); }

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

