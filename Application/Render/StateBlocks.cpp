#include "StateBlocks.h"
#include "Renderer.h"

namespace Nome
{

void CStaticRasterizerState::InitDeviceResources()
{
    if (!RastState)
    {
        auto* dev = GRenderer->GetGD()->GetDevice();
        dev->CreateRasterizerState(&Desc, RastState.GetAddressOf());
    }
}

void CStaticRasterizerState::ReleaseDeviceResources()
{
    RastState.Reset();
}

void CStaticDepthStencilState::InitDeviceResources()
{
    if (!StateObj)
    {
        auto* dev = GRenderer->GetGD()->GetDevice();
        dev->CreateDepthStencilState(&Desc, StateObj.GetAddressOf());
    }
}

void CStaticDepthStencilState::ReleaseDeviceResources()
{
    StateObj.Reset();
}

void CStaticSamplerState::InitDeviceResources()
{
    if (!StateObj)
    {
        auto* dev = GRenderer->GetGD()->GetDevice();
        dev->CreateSamplerState(&Desc, StateObj.GetAddressOf());
    }
}

void CStaticSamplerState::ReleaseDeviceResources()
{
    StateObj.Reset();
}

void CStaticBlendState::InitDeviceResources()
{
    if (!StateObj)
    {
        auto* dev = GRenderer->GetGD()->GetDevice();
        dev->CreateBlendState(&Desc, StateObj.GetAddressOf());
    }
}

void CStaticBlendState::ReleaseDeviceResources()
{
    StateObj.Reset();
}

CStaticRasterizerState DefaultRasterizerState{ CD3D11_DEFAULT() };
CStaticRasterizerState NoCullRasterizerState{ D3D11_FILL_SOLID, D3D11_CULL_NONE, true, 0, 0.0f, 0.0f, true, false, false, false };
CStaticRasterizerState NoCullBiasedRasterizerState{ D3D11_FILL_SOLID, D3D11_CULL_NONE, true, -1000, 0.0f, 0.0f, true, false, false, false };

CStaticDepthStencilState DefaultDepthStencilState{ CD3D11_DEFAULT() };
CStaticDepthStencilState PointDepthStencilState{true, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS,
    false, D3D11_DEFAULT_STENCIL_READ_MASK, D3D11_DEFAULT_STENCIL_WRITE_MASK,
    D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS,
    D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS };

CStaticSamplerState DefaultSamplerState{ CD3D11_DEFAULT() };
CStaticSamplerState Anisotropic16SamplerState{ [](CD3D11_SAMPLER_DESC& desc)
    {
        desc.Filter = D3D11_FILTER_ANISOTROPIC;
        desc.MaxAnisotropy = 16;
    }, CUseLambda() };

CStaticBlendState DefaultBlendState{ CD3D11_DEFAULT() };
CStaticBlendState PointBlendState{ [](CD3D11_BLEND_DESC& desc)
    {
        desc.RenderTarget[0] =
            {
                true,
                D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD,
                D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD,
                D3D11_COLOR_WRITE_ENABLE_ALL
            };
    }, CUseLambda() };

}
