#include "Light.h"
#include "SceneGraph.h"


namespace Nome::Scene {

DEFINE_META_OBJECT(CLight)
{
    BindNamedArgument(&CLight::R,"color", 0, 0);
    BindNamedArgument(&CLight::G,"color", 0, 1);
    BindNamedArgument(&CLight::B,"color", 0, 2);
}

void CLight::MarkDirty()
{
    // Mark this entity dirty
    Super::MarkDirty();
}

bool CLight::IsInstantiable() { return true; }

bool CLight::IsMesh() {
    return false;
}

CEntity* CLight::Instantiate(CSceneTreeNode* treeNode)
{
    SceneTreeNode = treeNode;
    return this;
}



void CLight::UpdateEntity() {
    if (!IsDirty())
        return;

    Super::UpdateEntity();
    LI.color = QColor(int(R.GetValue(0.0f) * 255), int(G.GetValue(0.0f) * 255), int(B.GetValue(0.0f) * 255));
    SetValid(true);
}

}
