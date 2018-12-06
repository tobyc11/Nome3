#pragma once

#include "IAppService.h"
#include "InputListener.h"

#include <vector>

namespace Nome
{

class CInputService : public IAppService
{
public:
	DEFINE_APP_SERVICE_TYPE_FLAGS(CInputService, ASF_EVENT_HOOK)

	void AddMouseListener(IMouseListener* listener);
	void RemoveMouseListener(IMouseListener* listener);

protected:
    int Setup() override;
    int FrameUpdate() override;
    int Cleanup() override;
	bool EventHook(void* event) override;

private:
	bool bFirstFrame = true;
    CMouseState MouseState;

	std::vector<IMouseListener*> MouseListeners;
};

}
