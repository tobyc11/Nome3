#include "App.h"

namespace Nome
{

int CApp::AppSetup()
{
    GApp->Init();
    return 0;
}

int CApp::AppSingleFrame()
{
	if (!GApp->EventLoopDriver)
	{
		printf("%s\n", "Couldn't find an event loop driver");
		return -1;
	}

	if (GApp->EventLoopDriver->EventLoopOnce())
		return StatusWannaQuit;
	else
		return 0;
}

int CApp::AppCleanup()
{
    GApp->ShutdownServices();
    GApp->Shutdown();
    return 0;
}

void CApp::ShutdownServices()
{
    for (auto iter = ServicesInBootOrder.rbegin();
         iter != ServicesInBootOrder.rend(); ++iter)
    {
        (*iter)->Cleanup();
        delete *iter;
    }
    ServicesInBootOrder.clear();
}

}
