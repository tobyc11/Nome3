#pragma once
#include "SwapChain.h"

namespace Nome
{

//Encapsulates a swapchain and its associated window
class CViewport
{
public:
	CViewport(CSwapChain* swapChain);

	float GetAspectRatio() const;
	float GetWidth() const;
	float GetHeight() const;

	void BindAndClear(ID3D11DeviceContext* ctx);

	void OnWindowResize(int width, int height);

private:
	void RecreateDSV();

	CSwapChain* SwapChain;

	ComPtr<ID3D11DepthStencilView> DepthBufferView;

	D3D11_VIEWPORT ViewportDesc;
};

} /* namespace Nome */
