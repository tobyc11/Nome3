#pragma once

#include "IAppService.h"

namespace Nome
{

//Base class for all event loop drivers
class CEventLoopDriver : public IAppService
{
    DEFINE_APP_SERVICE_TYPE_FLAGS(CEventLoopDriver, ASF_EVENT_LOOP_DRIVER)

protected:
	void RunFrameUpdate();
	void RunEventHook(void* event);
    void RunRender();
    void RunRenderPhase2();
};

}
