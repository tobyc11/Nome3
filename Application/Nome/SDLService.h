#pragma once

#include "EventLoopDriver.h"
#include <SDL_video.h>
#include <string>

namespace Nome
{

class CSDLService : public CEventLoopDriver
{
public:
    DEFINE_APP_SERVICE_TYPE_FLAGS(CSDLService, ASF_EVENT_LOOP_DRIVER)

	SDL_Window* Window;
	SDL_GLContext GLContext;

	std::string GLSLVersion;

protected:
    int Setup() override;
    int FrameUpdate() override;
    int Cleanup() override;
	bool EventLoopOnce() override;
};

}
