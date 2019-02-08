#pragma once
#include "GraphicsDevice.h"

namespace Nome
{

struct CUseLambda {};

class CStaticRasterizerState : private CRenderResource
{
public:
    template <typename... TArgs>
    CStaticRasterizerState(TArgs&&... args) : Desc(std::forward<TArgs>(args)...)
    {
    }

    ID3D11RasterizerState* const& Get()
    {
        InitDeviceResources();
        return RastState.Get();
    }

private:
    void InitDeviceResources() override;
    void ReleaseDeviceResources() override;

private:
    CD3D11_RASTERIZER_DESC Desc;
    ComPtr<ID3D11RasterizerState> RastState;
};

class CStaticDepthStencilState : private CRenderResource
{
public:
    template <typename... TArgs>
    CStaticDepthStencilState(TArgs&&... args) : Desc(std::forward<TArgs>(args)...)
    {
    }

    ID3D11DepthStencilState* const& Get()
    {
        InitDeviceResources();
        return StateObj.Get();
    }

private:
    void InitDeviceResources() override;
    void ReleaseDeviceResources() override;

private:
    CD3D11_DEPTH_STENCIL_DESC Desc;
    ComPtr<ID3D11DepthStencilState> StateObj;
};

class CStaticSamplerState : private CRenderResource
{
public:
    template <typename... TArgs>
    CStaticSamplerState(TArgs&&... args) : Desc(std::forward<TArgs>(args)...)
    {
    }

    //Allow a lambda to tailor the state based on the default state
    template <typename TLambda>
    CStaticSamplerState(TLambda lambda, CUseLambda) : Desc(CD3D11_DEFAULT())
    {
        lambda(Desc);
    }

    ID3D11SamplerState* const& Get()
    {
        InitDeviceResources();
        return StateObj.Get();
    }

private:
    void InitDeviceResources() override;
    void ReleaseDeviceResources() override;

private:
    CD3D11_SAMPLER_DESC Desc;
    ComPtr<ID3D11SamplerState> StateObj;
};

class CStaticBlendState : private CRenderResource
{
public:
    template <typename... TArgs>
    CStaticBlendState(TArgs&&... args) : Desc(std::forward<TArgs>(args)...)
    {
    }

    //Allow a lambda to tailor the state based on the default state
    template <typename TLambda>
    CStaticBlendState(TLambda lambda, CUseLambda) : Desc(CD3D11_DEFAULT())
    {
        lambda(Desc);
    }

    ID3D11BlendState* const& Get()
    {
        InitDeviceResources();
        return StateObj.Get();
    }

private:
    void InitDeviceResources() override;
    void ReleaseDeviceResources() override;

private:
    CD3D11_BLEND_DESC Desc;
    ComPtr<ID3D11BlendState> StateObj;
};

extern CStaticRasterizerState DefaultRasterizerState;
extern CStaticRasterizerState NoCullRasterizerState;
extern CStaticRasterizerState NoCullBiasedRasterizerState;

extern CStaticDepthStencilState DefaultDepthStencilState;
extern CStaticDepthStencilState PointDepthStencilState;

extern CStaticSamplerState DefaultSamplerState;
extern CStaticSamplerState Anisotropic16SamplerState;

extern CStaticBlendState DefaultBlendState;
extern CStaticBlendState PointBlendState;

}
