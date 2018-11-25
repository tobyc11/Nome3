#pragma once

#include "IAppService.h"

namespace Nome
{

//A thin layer that wraps a Viewport
class CRenderService : public IAppService
{
    DEFINE_APP_SERVICE_TYPE_FLAGS(CRenderService, ASF_NONE)

public:

protected:
    int Setup() override;

    int FrameUpdate() override
    {
        return 0;
    }

    int Cleanup() override
    {
        return 0;
    }

    void Render() override;
};

}
