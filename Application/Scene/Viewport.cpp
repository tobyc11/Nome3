#include "Viewport.h"

namespace Nome::Scene
{

DEFINE_META_OBJECT(CViewport)
{
    BindPositionalArgument(&CViewport::rect_x1, 1, 0);
    BindPositionalArgument(&CViewport::rect_x2, 1, 1);
    BindPositionalArgument(&CViewport::rect_y1, 1, 2);
    BindPositionalArgument(&CViewport::rect_y2, 1, 3);
}

void CViewport::UpdateEntity()
{
    if (!IsDirty())
        return;

    Super::UpdateEntity();
    viewport.setRect(rect_x1.GetValue(0.0), rect_x2.GetValue(0.0),
                     rect_y1.GetValue(1.0), rect_y2.GetValue(1.0));
    SetValid(true);
}

bool CViewport::IsMesh() {
    return false;
}

CEntity* CViewport::Instantiate(CSceneTreeNode* treeNode)
{
    SceneTreeNode = treeNode;
    return this;
}

bool CViewport::IsInstantiable() { return true; }

void CViewport::MarkDirty()
{
    // Mark this entity dirty
    Super::MarkDirty();
}



}
