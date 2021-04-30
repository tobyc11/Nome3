#include "Camera.h"


namespace Nome::Scene
{
    DEFINE_META_OBJECT(CCamera)
    {
        BindNamedArgument(&CCamera::left, "frustum", 0, 0);
        BindNamedArgument(&CCamera::right, "frustum", 0, 1);
        BindNamedArgument(&CCamera::bottom, "frustum", 0, 2);
        BindNamedArgument(&CCamera::top, "frustum", 0, 3);
        BindNamedArgument(&CCamera::nearPlane, "frustum", 0, 4);
        BindNamedArgument(&CCamera::farPlane, "frustum", 0, 5);
    }

    void CCamera::MarkDirty()
    {
        Super::MarkDirty();
    }

    void CCamera::UpdateEntity()
    {
        if (!IsDirty())
            return;
        CI.para[0] = left.GetValue(0.5);
        CI.para[1] = right.GetValue(-0.5);
        CI.para[2] = bottom.GetValue(-0.5);
        CI.para[3] = top.GetValue(0.5);
        CI.para[4] = nearPlane.GetValue(0.1);
        CI.para[5] = farPlane.GetValue(1000);
        Super::UpdateEntity();
        SetValid(true);
    }

    bool CCamera::IsMesh() {
        renderType = CAMERA;
        return false;
    }

    bool CCamera::IsInstantiable() {
        return true;
    }

    CEntity *CCamera::Instantiate(CSceneTreeNode *treeNode) {
        SceneTreeNode = treeNode;
        return this;
    }


}
