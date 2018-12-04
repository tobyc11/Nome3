#include "EventLoopDriver.h"
#include "App.h"

namespace Nome
{

void CEventLoopDriver::RunFrameUpdate()
{
	for (auto& svc : GApp->ServicesInBootOrder)
	{
		svc->FrameUpdate();
	}
}

void CEventLoopDriver::RunEventHook(void* event)
{
	for (auto& svc : GApp->EventSvcs)
	{
		if (svc->EventHook(event))
			return;
	}
}

void CEventLoopDriver::RunRender()
{
	for (auto& svc : GApp->RenderSvcs)
	{
		svc->Render();
	}
}

void CEventLoopDriver::RunRenderPhase2()
{
	for (auto& svc : GApp->RenderSvcs)
	{
		svc->RenderPhase2();
	}
}

}
