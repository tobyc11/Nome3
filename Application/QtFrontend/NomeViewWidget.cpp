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

	// Setup Dear ImGui binding
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

	ImGuiImpl = new CImGuiImplQt(this);
	ImGui_ImplDX11_Init(GRenderer->GetGD()->GetDevice(), GRenderer->GetGD()->GetImmediateContext());

	// Setup style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them. 
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple. 
	// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Read 'misc/fonts/README.txt' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("Resources/Fonts/Roboto-Medium.ttf", 16.0f);
	io.Fonts->AddFontFromFileTTF("Resources/Fonts/DroidSans.ttf", 16.0f);
	io.Fonts->AddFontFromFileTTF("Resources/Fonts/Cousine-Regular.ttf", 15.0f);
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != NULL);
}

CNomeViewWidget::~CNomeViewWidget()
{
	ImGui_ImplDX11_Shutdown();
	delete ImGuiImpl;
	ImGui::DestroyContext();
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
	ImGui::NewFrame();

	//Client should clear the backbuffers
	Client->Draw(this); //Also does scene update and ImGui update
	
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

void CNomeViewWidget::BindAndClear(ID3D11DeviceContext* ctx, const float* color)
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
	const float defaultColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	if (color)
		ctx->ClearRenderTargetView(rtv, color);
	else
		ctx->ClearRenderTargetView(rtv, defaultColor);
}

void CNomeViewWidget::mousePressEvent(QMouseEvent* event)
{
	if (Client->OnMousePress(this, event->button(), event->x(), event->y()))
		event->accept();
}

void CNomeViewWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if (Client->OnMouseRelease(this, event->button(), event->x(), event->y()))
		event->accept();
}

void CNomeViewWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (Client->OnMouseMove(this, event->x(), event->y()))
		event->accept();
}

void CNomeViewWidget::wheelEvent(QWheelEvent* event)
{
	if (Client->OnMouseWheel(this, event->angleDelta().y() / 8))
		event->accept();
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
