#include "Camera.h"


namespace Nome::Scene
{
    DEFINE_META_OBJECT(CCamera)
    {
        BindNamedArgument(&CCamera::para0, "frustum", 0, 0);
        BindNamedArgument(&CCamera::para1, "frustum", 0, 1);
        BindNamedArgument(&CCamera::para2, "frustum", 0, 2);
        BindNamedArgument(&CCamera::para3, "frustum", 0, 3);
        BindNamedArgument(&CCamera::para4, "frustum", 0, 4);
        BindNamedArgument(&CCamera::para5, "frustum", 0, 5);
        BindNamedArgument(&CCamera::translate_x, "ctranslate", 0, 0);
        BindNamedArgument(&CCamera::translate_y, "ctranslate", 0, 1);
        BindNamedArgument(&CCamera::translate_z, "ctranslate", 0, 2);
        BindNamedArgument(&CCamera::rotate_x, "crotate", 0, 0);
        BindNamedArgument(&CCamera::rotate_y, "crotate", 0, 1);
        BindNamedArgument(&CCamera::rotate_z, "crotate", 0, 2);
        BindNamedArgument(&CCamera::rotate_scale, "crotate", 1, 0);
    }

    void CCamera::MarkDirty()
    {
        Super::MarkDirty();
    }

    void CCamera::UpdateEntity()
    {
        para[0] = para0.GetValue(0.5);
        para[1] = para1.GetValue(-0.5);
        para[2] = para2.GetValue(-0.5);
        para[3] = para3.GetValue(0.5);
        para[4] = para4.GetValue(0.1);
        para[5] = para5.GetValue(1000);
        rotation.setVector(rotate_x.GetValue(0), rotate_y.GetValue(0), rotate_z.GetValue(0));
        rotation.setScalar(rotate_scale.GetValue(0));
        translation.setX(translate_x.GetValue(0.0));
        translation.setY(translate_y.GetValue(0.0));
        translation.setZ(translate_z.GetValue(0.0));
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
