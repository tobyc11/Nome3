#pragma once

#include "Material.h"

#include <Color.h>

namespace Nome::Render
{

class CMatteMaterial : public CMaterial
{
public:
    explicit CMatteMaterial(tc::Color kd);
    explicit CMatteMaterial(RHI::CImageView* imageView);

    void ChooseShaders(CShaderCombiner& combiner) const override;
    void BindPipelineArgs(RHI::CDrawTemplate& drawTemplate) const override;

private:
    tc::Color Kd;
    tc::sp<RHI::CImageView> KdTextureView;
    tc::sp<RHI::CSampler> KdSampler;
    static tc::sp<RHI::CBuffer> MaterialParams;
};

} /* namespace Nome::Render */
