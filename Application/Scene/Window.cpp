#include "Window.h"

namespace Nome::Scene
{
    DEFINE_META_OBJECT(CWindow)
    {
        BindNamedArgument(&CWindow::origin_x, "origin", 0, 0);
        BindNamedArgument(&CWindow::origin_y, "origin", 0, 1);
        BindNamedArgument(&CWindow::width, "size", 0, 0);
        BindNamedArgument(&CWindow::length, "size", 0, 1);
        BindNamedArgument(&CWindow::R, "Window", 0, 0);
        BindNamedArgument(&CWindow::G, "Window", 0, 1);
        BindNamedArgument(&CWindow::R, "Window", 0, 2);


    }

    void CWindow::MarkDirty()
    {
        Super::MarkDirty();
    }

    void CWindow::UpdateEntity()
    {
        Super::UpdateEntity();
        Background.setRedF(R.GetValue(0.3));
        Background.setGreenF(G.GetValue(0.3));
        Background.setBlueF(B.GetValue(0.3));
        window.setRect(origin_x.GetValue(0.0), origin_y.GetValue(0.0),
                         width.GetValue(1.0), length.GetValue(1.0));
        SetValid(true);
    }

    bool CWindow::IsMesh() {
        renderType = WINDOW;
        return false;
    }

    bool CWindow::IsInstantiable() {
        return true;
    }

    CEntity *CWindow::Instantiate(CSceneTreeNode *treeNode) {
        SceneTreeNode = treeNode;
        return this;
    }


}
