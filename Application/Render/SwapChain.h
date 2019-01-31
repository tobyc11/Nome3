#pragma once
#include "Renderer.h"
#include "IncludeD3D11.h"

namespace Nome
{

class CSwapChain
{
	friend class CViewport;

public:
	CSwapChain(CRenderer* renderer, HWND hWnd);
	~CSwapChain();

	//Should be called by the window
	int ResizeBackbuffer(int width, int height);

	void ClearRenderTarget(const float* color);
	void Present();

	int GetWidth() const;
	int GetHeight() const;

	ID3D11RenderTargetView* GetRenderTargetView() const { return RenderTargetView; }

private:
	CRenderer* Renderer;

	ID3D11Device* D3dDevice = nullptr;
	ID3D11Device1* D3dDevice1 = nullptr;
	ID3D11DeviceContext* ImmediateContext = nullptr;
	ID3D11DeviceContext1* ImmediateContext1 = nullptr;
	IDXGISwapChain* SwapChain = nullptr;
	IDXGISwapChain1* SwapChain1 = nullptr;
	ID3D11RenderTargetView* RenderTargetView = nullptr;
};

} /* namespace Nome */
