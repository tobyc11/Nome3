#include "SwapChain.h"
#include "GraphicsDevice.h"

namespace Nome
{

CSwapChain::CSwapChain(CRenderer* renderer, HWND hWnd) : Renderer(renderer)
{
    HRESULT hr = S_OK;

    RECT windowClient;
    GetClientRect(hWnd, &windowClient);
    int width = windowClient.right;
    int height = windowClient.bottom;

    //Prepare GD
    D3dDevice = Renderer->GetGD()->GetDevice();
    D3dDevice->AddRef();
    D3dDevice->GetImmediateContext(&ImmediateContext);

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
        throw CGraphicsException("Create swapchain failed");

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
        throw CGraphicsException("Create swapchain failed");

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
    if (FAILED(hr))
        throw CGraphicsException("Create swapchain failed");

    hr = D3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &RenderTargetView);
    pBackBuffer->Release();
    if (FAILED(hr))
        throw CGraphicsException("Create swapchain failed");

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
}

CSwapChain::~CSwapChain()
{
    if (ImmediateContext) ImmediateContext->ClearState();

    if (RenderTargetView) RenderTargetView->Release();
    if (SwapChain1) SwapChain1->Release();
    if (SwapChain) SwapChain->Release();
    if (ImmediateContext1) ImmediateContext1->Release();
    if (ImmediateContext) ImmediateContext->Release();
    if (D3dDevice1) D3dDevice1->Release();
    if (D3dDevice) D3dDevice->Release();
}

int CSwapChain::ResizeBackbuffer(int width, int height)
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

void CSwapChain::ClearRenderTarget(const float* color)
{
    ImmediateContext->OMSetRenderTargets(1, &RenderTargetView, nullptr);
    ImmediateContext->ClearRenderTargetView(RenderTargetView, color);
}

void CSwapChain::Present()
{
    SwapChain->Present(0, 0);
}

int CSwapChain::GetWidth() const
{
    ID3D11Texture2D* pBackBuffer;
    SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
    D3D11_TEXTURE2D_DESC desc;
    pBackBuffer->GetDesc(&desc);
    pBackBuffer->Release();
    return desc.Width;
}

int CSwapChain::GetHeight() const
{
    ID3D11Texture2D* pBackBuffer;
    SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
    D3D11_TEXTURE2D_DESC desc;
    pBackBuffer->GetDesc(&desc);
    pBackBuffer->Release();
    return desc.Height;
}

} /* namespace Nome */
