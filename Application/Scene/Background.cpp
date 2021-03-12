#include "Background.h"

namespace Nome::Scene
{
    DEFINE_META_OBJECT(CBackground)
    {
        BindPositionalArgument(&CBackground::R, 1, 0);
        BindPositionalArgument(&CBackground::G, 1, 1);
        BindPositionalArgument(&CBackground::B, 1, 2);
    }

    void CBackground::MarkDirty()
    {
        Super::MarkDirty();
    }

    void CBackground::UpdateEntity()
    {
        Super::UpdateEntity();
        background.setRedF(R.GetValue(0.3));
        background.setGreenF(G.GetValue(0.3));
        background.setBlueF(B.GetValue(0.3));
        SetValid(true);
    }

    bool CBackground::IsMesh() {
        renderType = BACKGROUND;
        return false;
    }

    bool CBackground::IsInstantiable() {
        return true;
    }

    CEntity *CBackground::Instantiate(CSceneTreeNode *treeNode) {
        SceneTreeNode = treeNode;
        return this;
    }


}
