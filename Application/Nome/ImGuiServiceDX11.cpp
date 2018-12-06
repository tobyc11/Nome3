#include "ImGuiServiceDX11.h"
#include "App.h"
#include "SDLService.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_dx11.h"

#include <SDL.h>

#include <filesystem>

static bool show_demo_window = false;
static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

namespace Nome
{

int CImGuiServiceDX11::Setup()
{
	// Setup Dear ImGui binding
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

	CSDLService* SDLSrv = GApp->GetService<CSDLService>();
	CGraphicsDevice* GD = SDLSrv->RenderContext->GetGraphicsDevice();
	ImGui_ImplSDL2_InitForOpenGL(SDLSrv->Window, nullptr);
    ImGui_ImplDX11_Init(GD->GetDevice(), GD->GetImmediateContext());

	// Setup style
	ImGui::StyleColorsDark();

	// Load Fonts
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);

    return 0;
}

int CImGuiServiceDX11::FrameUpdate()
{
	CSDLService* SDLSrv = GApp->GetService<CSDLService>();

	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplSDL2_NewFrame(SDLSrv->Window);
	ImGui::NewFrame();

	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		ImGui::Begin("Info");

		ImGui::Checkbox("Demo Window", &show_demo_window);

		ImGui::ColorEdit3("clear color", (float*)&clear_color);

		ImGui::TextWrapped("PATH=%s", getenv("PATH"));
		ImGui::TextWrapped("CWD=%s", std::filesystem::current_path().string().c_str());

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}
    return 0;
}

int CImGuiServiceDX11::Cleanup()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
    return 0;
}

bool CImGuiServiceDX11::EventHook(void* event)
{
	SDL_Event* e = static_cast<SDL_Event*>(event);

	ImGuiIO& io = ImGui::GetIO();

	ImGui_ImplSDL2_ProcessEvent(e);

	//Dispatch the event to services
	if ((e->type & 0x400) && io.WantCaptureMouse)
		return true;
	if ((e->type & 0x300) && io.WantCaptureKeyboard)
		return true;

	return false;
}

void CImGuiServiceDX11::Render()
{
	CSDLService* SDLSrv = GApp->GetService<CSDLService>();
	ImGuiIO& io = ImGui::GetIO();

	// Rendering
	ImGui::Render();

	SDLSrv->RenderContext->ClearRenderTarget((float*)&clear_color);
}

void CImGuiServiceDX11::RenderPhase2()
{
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

}