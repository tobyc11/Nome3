#pragma once

#include "Core/ShaderCombiner.h"
#include <DrawTemplate.h>

namespace Nome::Render
{

class CMaterial
{
public:
    virtual ~CMaterial() = default;
    virtual void ChooseShaders(CShaderCombiner& combiner) const = 0;
    virtual void BindPipelineArgs(RHI::CDrawTemplate& drawTemplate) const = 0;
};

} /* namespace Nome::Render */
