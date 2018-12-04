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
	for (auto& iter : GApp->ServicesInBootOrder)
	{
		if (iter->Flags() & ASF_EVENT_LOOP_DRIVER)
		{
			if (iter->EventLoopOnce())
				return StatusWannaQuit;
			else
				return 0;
		}
	}

	printf("%s\n", "Couldn't find an event loop driver");
    return -1;
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
