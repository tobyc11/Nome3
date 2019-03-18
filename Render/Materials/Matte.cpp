#include "Matte.h"

#include <RHIInstance.h>

namespace Nome::Render
{

tc::sp<RHI::CBuffer> CMatteMaterial::MaterialParams;

CMatteMaterial::CMatteMaterial(tc::Color kd)
    : Kd(kd)
{
    if (!MaterialParams)
        MaterialParams = RHI::CInstance::Get().GetCurrDevice()->CreateBuffer(sizeof(float) * 4, RHI::EBufferUsageFlags::ConstantBuffer);
}

CMatteMaterial::CMatteMaterial(RHI::CImageView* imageView)
    : KdTextureView(imageView)
{
    RHI::CSamplerDesc desc;
    KdSampler = RHI::CInstance::Get().GetCurrDevice()->CreateSampler(desc);
}

void CMatteMaterial::ChooseShaders(CShaderCombiner& combiner) const
{
    if (!KdTextureView)
        combiner.SetMaterial("MatteMaterialC");
    else
        combiner.SetMaterial("MatteMaterialI");
}

void CMatteMaterial::BindPipelineArgs(RHI::CDrawTemplate& drawTemplate) const
{
    RHI::CDepthStencilDesc ddesc;
    ddesc.DepthTestEnable = true;
    ddesc.DepthWriteEnable = true;
    ddesc.DepthCompareOp = RHI::ECompareOp::Less;
    ddesc.StencilTestEnable = false;
    RHI::CStencilOpState sop;
    sop.SetCompareOp(RHI::ECompareOp::Always)
        .SetPassOp(RHI::EStencilOp::Keep)
        .SetFailOp(RHI::EStencilOp::Keep)
        .SetDepthFailOp(RHI::EStencilOp::Keep)
        .SetCompareMask(0xff)
        .SetWriteMask(0xff)
        .SetReference(0xff);
    ddesc.Front = sop;
    ddesc.Back = sop;
    ddesc.DepthBoundsTestEnable = false;
    ddesc.MinDepthBounds = .0f;
    ddesc.MaxDepthBounds = 1.0f;
    drawTemplate.SetDepthStencilDesc(ddesc);

    if (!KdTextureView)
    {
        RHI::CBufferUpdateRequest req;
        req.Buffer = MaterialParams;
        req.Size = sizeof(float) * 4;
        req.Data = &Kd.r;
        drawTemplate.GetBufferUpdateReqs().push_back(req);
        drawTemplate.GetPipelineArguments().Add("material"_hash, MaterialParams);
    }
    else
    {
        drawTemplate.GetPipelineArguments().Add("material_Texture_Image"_hash, KdTextureView);
        drawTemplate.GetPipelineArguments().Add("material_Texture_Sampler"_hash, KdSampler);
    }
}

} /* namespace Nome::Render */
