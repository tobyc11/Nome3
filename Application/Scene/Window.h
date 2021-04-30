#pragma once
#include <QColor>
#include <QRect>
#include "Entity.h"


namespace Nome::Scene
{

    class CWindow : public CEntity {
        DEFINE_INPUT(float, origin_x) { MarkDirty(); }
        DEFINE_INPUT(float, origin_y) { MarkDirty(); }
        DEFINE_INPUT(float, width) { MarkDirty(); }
        DEFINE_INPUT(float, length) { MarkDirty(); }
        DEFINE_INPUT(float, R) { MarkDirty(); }
        DEFINE_INPUT(float, G) { MarkDirty(); }
        DEFINE_INPUT(float, B) { MarkDirty(); }
        void MarkDirty() override;
        void UpdateEntity() override;

    public:
        DECLARE_META_CLASS(CWindow, CEntity);
        CWindow() = default;
        explicit CWindow(const std::string& name)
                : CEntity(std::move(name))
        {
        }

        bool IsMesh() override;
        bool IsInstantiable() override;
        CEntity* Instantiate(CSceneTreeNode* treeNode) override;

    public:
        QColor Background;
        QRectF window;

    private:
        CSceneTreeNode* SceneTreeNode;
    };

}
