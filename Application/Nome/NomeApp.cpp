#include "NomeApp.h"
#include "InputService.h"
#include "RenderService.h"

namespace Nome
{

void CNomeApp::Init()
{
    BootService<CInputService>();
    BootService<CRenderService>();
}

void CNomeApp::Shutdown()
{
}

}
