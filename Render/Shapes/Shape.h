#pragma once

#include "Core/ShaderCombiner.h"
#include <BoundingBox.h>
#include <DrawTemplate.h>

namespace Nome::Render
{

class CShape
{
public:
    virtual ~CShape() = default;
    virtual tc::BoundingBox ObjectBound() const = 0;
    virtual void ChooseShaders(CShaderCombiner& combiner) const = 0;
    virtual void BindPipelineArgs(RHI::CDrawTemplate& drawTemplate) const = 0;
};

} /* namespace Nome::Render */
