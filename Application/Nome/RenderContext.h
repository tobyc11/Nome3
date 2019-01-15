#pragma once
#include "Render/GraphicsDevice.h"
#include <SDL_video.h>

namespace Nome
{

//Abstracts over all the rendering related aspects of a window, here is the Direct3D version
//OpenGL counterpart should be much eazier to implement, e.g., SDL_GL_CreateContext
class CRenderContext
{
public:
	CRenderContext(SDL_Window* window);
	int Init();
	int Cleanup();

	//Should be called by the window
	int ResizeBackbuffer(int width, int height);

	CGraphicsDevice* GetGraphicsDevice();

	void ClearRenderTarget(const float* color);
	void Present();

private:
	SDL_Window* Window;

	D3D_DRIVER_TYPE DriverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL FeatureLevel = D3D_FEATURE_LEVEL_11_0;
	ID3D11Device* D3dDevice = nullptr;
	ID3D11Device1* D3dDevice1 = nullptr;
	ID3D11DeviceContext* ImmediateContext = nullptr;
	ID3D11DeviceContext1* ImmediateContext1 = nullptr;
	IDXGISwapChain* SwapChain = nullptr;
	IDXGISwapChain1* SwapChain1 = nullptr;
	ID3D11RenderTargetView* RenderTargetView = nullptr;

	CGraphicsDevice* GD;
};

} /* namespace Nome */
