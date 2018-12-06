#include "RenderContext.h"

#include <SDL.h>
#include <SDL_syswm.h>

namespace Nome
{

CRenderContext::CRenderContext(SDL_Window* window) : Window(window)
{
}

int CRenderContext::Init()
{
	HRESULT hr = S_OK;

	int width;
	int height;
	SDL_GetWindowSize(Window, &width, &height);

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		DriverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDevice(nullptr, DriverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &D3dDevice, &FeatureLevel, &ImmediateContext);

		if (hr == E_INVALIDARG)
		{
			// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
			hr = D3D11CreateDevice(nullptr, DriverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
				D3D11_SDK_VERSION, &D3dDevice, &FeatureLevel, &ImmediateContext);
		}

		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
		return hr;

	//Prepare GD
	GD = new CGraphicsDevice(D3dDevice);

	// Obtain DXGI factory from device (since we used nullptr for pAdapter above)
	IDXGIFactory1* dxgiFactory = nullptr;
	{
		IDXGIDevice* dxgiDevice = nullptr;
		hr = D3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
		if (SUCCEEDED(hr))
		{
			IDXGIAdapter* adapter = nullptr;
			hr = dxgiDevice->GetAdapter(&adapter);
			if (SUCCEEDED(hr))
			{
				hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
				adapter->Release();
			}
			dxgiDevice->Release();
		}
	}
	if (FAILED(hr))
		return hr;

	//Grab hWnd from SDL
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(Window, &wmInfo);
	HWND hWnd = wmInfo.info.win.window;

	// Create swap chain
	IDXGIFactory2* dxgiFactory2 = nullptr;
	hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));
	if (dxgiFactory2)
	{
		// DirectX 11.1 or later
		hr = D3dDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&D3dDevice1));
		if (SUCCEEDED(hr))
		{
			(void)ImmediateContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&ImmediateContext1));
		}

		DXGI_SWAP_CHAIN_DESC1 sd = {};
		sd.Width = width;
		sd.Height = height;
		sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;
		
		hr = dxgiFactory2->CreateSwapChainForHwnd(D3dDevice, hWnd, &sd, nullptr, nullptr, &SwapChain1);
		if (SUCCEEDED(hr))
		{
			hr = SwapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&SwapChain));
		}

		dxgiFactory2->Release();
	}
	else
	{
		// DirectX 11.0 systems
		DXGI_SWAP_CHAIN_DESC sd = {};
		sd.BufferCount = 1;
		sd.BufferDesc.Width = width;
		sd.BufferDesc.Height = height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hWnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;

		hr = dxgiFactory->CreateSwapChain(D3dDevice, &sd, &SwapChain);
	}

	// Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
	dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

	dxgiFactory->Release();

	if (FAILED(hr))
		return hr;

	// Create a render target view
	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
	if (FAILED(hr))
		return hr;

	hr = D3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &RenderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr))
		return hr;

	ImmediateContext->OMSetRenderTargets(1, &RenderTargetView, nullptr);

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	ImmediateContext->RSSetViewports(1, &vp);
	return 0;
}

int CRenderContext::Cleanup()
{
	delete GD; GD = nullptr;

	if (ImmediateContext) ImmediateContext->ClearState();

	if (RenderTargetView) RenderTargetView->Release();
	if (SwapChain1) SwapChain1->Release();
	if (SwapChain) SwapChain->Release();
	if (ImmediateContext1) ImmediateContext1->Release();
	if (ImmediateContext) ImmediateContext->Release();
	if (D3dDevice1) D3dDevice1->Release();
	if (D3dDevice) D3dDevice->Release();
	return 0;
}

int CRenderContext::ResizeBackbuffer(int width, int height)
{
	//Release the RTV
	if (RenderTargetView) { RenderTargetView->Release(); RenderTargetView = NULL; }

	//Resize the backbuffer
	SwapChain->ResizeBuffers(0, (UINT)width, (UINT)height, DXGI_FORMAT_UNKNOWN, 0);

	//Create the RTV
	ID3D11Texture2D* pBackBuffer;
	SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
	D3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &RenderTargetView);
	pBackBuffer->Release();
	return 0;
}

CGraphicsDevice* CRenderContext::GetGraphicsDevice()
{
	return GD;
}

void CRenderContext::ClearRenderTarget(const float* color)
{
	ImmediateContext->OMSetRenderTargets(1, &RenderTargetView, nullptr);
	ImmediateContext->ClearRenderTargetView(RenderTargetView, color);
}

void CRenderContext::Present()
{
	SwapChain->Present(1, 0);
}

} /* namespace Nome */
