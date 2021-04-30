#include "Viewport.h"

namespace Nome::Scene
{

DEFINE_META_OBJECT(CViewport)
{
    BindNamedArgument(&CViewport::origin_x, "origin", 0, 0);
    BindNamedArgument(&CViewport::origin_y, "origin", 0, 1);
    BindNamedArgument(&CViewport::width, "size", 0, 0);
    BindNamedArgument(&CViewport::length, "size", 0, 1);
}

void CViewport::UpdateEntity()
{
    if (!IsDirty())
        return;

    Super::UpdateEntity();
    viewport.setRect(origin_x.GetValue(0.0), origin_y.GetValue(0.0),
                     width.GetValue(1.0), length.GetValue(1.0));
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
