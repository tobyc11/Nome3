#include "Material.h"
#include "Renderer.h"
#include "GraphicsDevice.h"

namespace Nome
{

bool CMaterial::IsWireframe() const
{
    return bIsWireframe;
}

void CMaterial::SetWireframe(bool value)
{
    bIsWireframe = value;
    bStatesDirty = true;
}

void CMaterial::Bind(ID3D11DeviceContext* ctx)
{
    if (bStatesDirty)
    {
        auto* dev = GRenderer->GetGD()->GetDevice();
        {
            D3D11_RASTERIZER_DESC desc;
            desc.FillMode = D3D11_FILL_SOLID;
            desc.CullMode = D3D11_CULL_NONE;
            desc.FrontCounterClockwise = true;
            desc.DepthBias = 0;
            desc.DepthBiasClamp = 0.0f;
            desc.SlopeScaledDepthBias = 0.0f;
            desc.DepthClipEnable = false;
            desc.ScissorEnable = false;
            desc.MultisampleEnable = false;
            desc.AntialiasedLineEnable = false;
            dev->CreateRasterizerState(&desc, Rast.GetAddressOf());
        }
        {
            D3D11_DEPTH_STENCIL_DESC desc = {};
            desc.DepthEnable = true;
            desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
            desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
            desc.StencilEnable = false;
            desc.FrontFace.StencilFailOp = desc.FrontFace.StencilDepthFailOp = desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
            desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
            desc.BackFace = desc.FrontFace;
            dev->CreateDepthStencilState(&desc, DepthStencil.GetAddressOf());
        }
        {
            D3D11_BLEND_DESC desc = {};
            desc.RenderTarget[0].BlendEnable = false;
            desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
            dev->CreateBlendState(&desc, Blend.GetAddressOf());
        }
        bStatesDirty = false;
    }
    ctx->RSSetState(Rast.Get());
    ctx->OMSetDepthStencilState(DepthStencil.Get(), 1);
    float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    UINT sampleMask = 0xffffffff;
    ctx->OMSetBlendState(Blend.Get(), blendFactor, sampleMask);
}

}
