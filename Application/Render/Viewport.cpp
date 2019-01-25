#include "Viewport.h"
#include "GraphicsDevice.h"

namespace Nome
{

CViewport::CViewport(CSwapChain* swapChain) : SwapChain(swapChain)
{
	ViewportDesc.TopLeftX = 0;
	ViewportDesc.TopLeftY = 0;
	ViewportDesc.Width = SwapChain->GetWidth();
	ViewportDesc.Height = SwapChain->GetHeight();
	ViewportDesc.MinDepth = 0.0f;
	ViewportDesc.MaxDepth = 1.0f;
}

float CViewport::GetAspectRatio() const
{
	return (float)ViewportDesc.Width / ViewportDesc.Height;
}

float CViewport::GetWidth() const
{
	return ViewportDesc.Width;
}

float CViewport::GetHeight() const
{
	return ViewportDesc.Height;
}

void CViewport::BindAndClear(ID3D11DeviceContext* ctx)
{
	if (!DepthBufferView)
		RecreateDSV();

	ctx->RSSetViewports(1, &ViewportDesc);
	ctx->OMSetRenderTargets(1, &SwapChain->RenderTargetView, DepthBufferView.Get());
	ctx->ClearDepthStencilView(DepthBufferView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void CViewport::OnWindowResize(int width, int height)
{
	ViewportDesc.Width = width;
	ViewportDesc.Height = height;
	RecreateDSV();
}

void CViewport::RecreateDSV()
{
	ID3D11Texture2D* pDepthStencil = nullptr;
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = ViewportDesc.Width;
	desc.Height = ViewportDesc.Height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	HRESULT hr = GRenderer->GetGD()->GetDevice()->CreateTexture2D(&desc, nullptr, &pDepthStencil);
	if (!SUCCEEDED(hr))
		throw std::runtime_error("Cannot create depth buffer!");

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	GRenderer->GetGD()->GetDevice()->CreateDepthStencilView(pDepthStencil, &dsvDesc, DepthBufferView.GetAddressOf());
	pDepthStencil->Release();
}

} /* namespace Nome */
