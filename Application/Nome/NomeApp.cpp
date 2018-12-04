#include "NomeApp.h"
#include "Nome/SDLService.h"
#include "Nome/ImGuiService.h"
#include "InputService.h"
#include "RenderService.h"

namespace Nome
{

void CNomeApp::Init()
{
	BootService<CSDLService>();
	BootService<CImGuiService>();
    BootService<CInputService>();
    BootService<CRenderService>();
}

void CNomeApp::Shutdown()
{
}

}
