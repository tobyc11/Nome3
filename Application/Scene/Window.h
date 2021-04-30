#pragma once
#include "Entity.h"
#include <QColor>


namespace Nome::Scene
{
    class CBackground : public CEntity {
        DEFINE_INPUT(float, R) { MarkDirty(); }
        DEFINE_INPUT(float, G) { MarkDirty(); }
        DEFINE_INPUT(float, B) { MarkDirty(); }
        void MarkDirty() override;
        void UpdateEntity() override;

    public:
        DECLARE_META_CLASS(CBackground, CEntity);
        CBackground() = default;
        explicit CBackground(const std::string& name)
                : CEntity(std::move(name))
        {
        }

        bool IsMesh() override;
        bool IsInstantiable() override;
        CEntity* Instantiate(CSceneTreeNode* treeNode) override;

    public:
        QColor background;

    private:
        CSceneTreeNode* SceneTreeNode;
    };

}
