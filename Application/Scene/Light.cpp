#include "Light.h"

namespace Nome::Scene {

DEFINE_META_OBJECT(CLight)
{
    BindNamedArgument(&CLight::type, "type", 0);
    BindNamedArgument(&CLight::R,"color", 0, 0);
    BindNamedArgument(&CLight::G,"color", 0, 1);
    BindNamedArgument(&CLight::B,"color", 0, 2);

}

void CLight::MarkDirty()
{
    // Mark this entity dirty
    Super::MarkDirty();

}


void CLight::UpdateEntity() {
    if (!IsDirty())
        return;

    Super::UpdateEntity();
    LI.color = { R.GetValue(0.0f), G.GetValue(0.0f), B.GetValue(0.0f) };
    LI.type = type.GetValue("NOME_AMBIENT");
    LI.name = GetName();
    SetValid(true);
    //TODO:


}





}
