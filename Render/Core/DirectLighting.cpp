#include "DirectLighting.h"

namespace Nome::Render
{

void CDirectLightingModel::ChooseShaders(CShaderCombiner& combiner) const
{
    combiner.SetIntegrator("DirectLighting");
}

void CDirectLightingModel::BindToDrawTemplate(
    RHI::CDrawTemplate& drawTemplate) const {}

} // namespace Nome::Render
