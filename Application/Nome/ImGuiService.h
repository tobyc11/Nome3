#pragma once

#include "IAppService.h"

namespace Nome
{

class CImGuiService : public IAppService
{
public:
    DEFINE_APP_SERVICE_TYPE_FLAGS(CImGuiService, ASF_EVENT_HOOK | ASF_RENDER)

protected:
    int Setup() override;
    int FrameUpdate() override;
    int Cleanup() override;
	bool EventHook(void* event) override;
	void Render() override;
	void RenderPhase2() override;
};

}
