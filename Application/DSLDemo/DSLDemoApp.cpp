#include "DSLDemoApp.h"
#include "Nome/InputService.h"
#include "Nome/RenderService.h"

namespace Nome
{

REGISTER_APPLICATION(CDSLDemoApp)

void CDSLDemoApp::Init()
{
    BootService<CInputService>();
    BootService<CRenderService>();
}

void CDSLDemoApp::Shutdown()
{
}

}
