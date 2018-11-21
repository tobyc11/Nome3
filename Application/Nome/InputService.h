#pragma once

#include "IAppService.h"

namespace Nome
{

struct CMouseState
{
    int x, y;
    int dx, dy;
};

class CInputService : public IAppService
{
public:
    DEFINE_APP_SERVICE_TYPE_FLAGS(CInputService, ASF_EVENT_HOOK)

protected:
    int Setup() override;
    int FrameUpdate() override;
    int Cleanup() override;
    void EventHook(void* event) override;

private:
    CMouseState MouseState;
};

}
