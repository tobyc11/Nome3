#pragma once

#include "ShaderCombiner.h"

#include <DrawTemplate.h>

namespace Nome::Render
{

class CDirectLightingModel
{
public:
    void ChooseShaders(CShaderCombiner& combiner) const;
    void BindToDrawTemplate(RHI::CDrawTemplate& drawTemplate) const;
};

} // namespace Nome::Render
