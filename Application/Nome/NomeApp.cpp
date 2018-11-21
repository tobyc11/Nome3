#include "NomeApp.h"
#include "InputService.h"
#include "RenderService.h"

namespace Nome
{

REGISTER_APPLICATION(CNomeApp)

void CNomeApp::Init()
{
    BootService<CInputService>();
    BootService<CRenderService>();
}

void CNomeApp::Shutdown()
{
}

}
