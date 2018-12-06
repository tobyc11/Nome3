#include "DSLDemoApp.h"
#include "Nome/SDLService.h"
#include "Nome/ImGuiServiceDX11.h"
#include "Nome/InputService.h"
#include "Nome/RenderService.h"
#include "DSLDemoCoreService.h"

namespace Nome
{

REGISTER_APPLICATION(CDSLDemoApp)

void CDSLDemoApp::Init()
{
	CSDLService::SetOpenGLOff();
	BootService<CSDLService>();
	BootService<CImGuiServiceDX11>();
    BootService<CInputService>();
    //BootService<CRenderService>();

    BootService<CDSLDemoCoreService>();
}

void CDSLDemoApp::Shutdown()
{
}

}
