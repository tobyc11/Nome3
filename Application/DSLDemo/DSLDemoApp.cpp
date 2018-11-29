#include "DSLDemoApp.h"
#include "Nome/InputService.h"
#include "Nome/RenderService.h"
#include "DSLDemoCoreService.h"

namespace Nome
{

REGISTER_APPLICATION(CDSLDemoApp)

void CDSLDemoApp::Init()
{
    BootService<CInputService>();
    BootService<CRenderService>();

    BootService<CDSLDemoCoreService>();
}

void CDSLDemoApp::Shutdown()
{
}

}
