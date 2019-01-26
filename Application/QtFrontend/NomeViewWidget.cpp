#include "NomeViewWidget.h"
#include <Render/Renderer.h>
#include <Render/GraphicsDevice.h>

#include <imgui.h>
#include <imgui_impl_dx11.h>

namespace Nome
{

CNomeViewWidget::CNomeViewWidget(QWidget* parent, CViewportClient* client) : CD3DWidget(parent), CViewport(client)
{
	setMouseTracking(true);
	HWND hWnd = (HWND) this->winId();
	SwapChain = new CSwapChain(GRenderer, hWnd);
	ImGuiImpl = new CImGuiImplQt(this);

	ImGui_ImplDX11_Init(GRenderer->GetGD()->GetDevice(), GRenderer->GetGD()->GetImmediateContext());
}

CNomeViewWidget::~CNomeViewWidget()
{
	ImGui_ImplDX11_Shutdown();

	delete ImGuiImpl;
	delete SwapChain;
}

void CNomeViewWidget::UpdateAndDraw()
{
	/* Frame order:
		 Handle input events
		 Update scene
		 Render
	*/
	ImGui_ImplDX11_NewFrame();
	ImGuiImpl->NewFrame();

	//Client should clear the backbuffers
	Client->Draw(this);
	
	// Render ImGui
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	SwapChain->Present();
}

float CNomeViewWidget::GetAspectRatio()
{
	return GetWidth() / GetHeight();
}

float CNomeViewWidget::GetWidth()
{
	return (float)width();
}

float CNomeViewWidget::GetHeight()
{
	return (float)height();
}

void CNomeViewWidget::BindAndClear(ID3D11DeviceContext* ctx)
{
	if (!DepthBufferView)
		RecreateDSV();

	D3D11_VIEWPORT desc;
	desc.TopLeftX = 0;
	desc.TopLeftY = 0;
	desc.Width = GetWidth();
	desc.Height = GetHeight();
	desc.MinDepth = 0.0f;
	desc.MaxDepth = 1.0f;

	auto* rtv = SwapChain->GetRenderTargetView();
	ctx->RSSetViewports(1, &desc);
	ctx->OMSetRenderTargets(1, &rtv, DepthBufferView.Get());
	ctx->ClearDepthStencilView(DepthBufferView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	const float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	ctx->ClearRenderTargetView(rtv, color);
}

void CNomeViewWidget::mousePressEvent(QMouseEvent* event)
{
	Client->OnMousePress(this, event->button(), event->x(), event->y());
}

void CNomeViewWidget::mouseReleaseEvent(QMouseEvent* event)
{
	Client->OnMouseRelease(this, event->button(), event->x(), event->y());
}

void CNomeViewWidget::mouseMoveEvent(QMouseEvent* event)
{
	Client->OnMouseMove(this, event->x(), event->y());
}

void CNomeViewWidget::wheelEvent(QWheelEvent* event)
{
	Client->OnMouseWheel(this, event->angleDelta().y() / 8);
}

void CNomeViewWidget::RecreateDSV()
{
	ID3D11Texture2D* pDepthStencil = nullptr;
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = GetWidth();
	desc.Height = GetHeight();
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

}
