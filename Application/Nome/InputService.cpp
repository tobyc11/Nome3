#include "InputService.h"

#include <SDL.h>

namespace Nome
{

int CInputService::Setup()
{
    SDL_GetMouseState(&MouseState.x, &MouseState.y);
    MouseState.dx = 0;
    MouseState.dy = 0;
    return 0;
}

int CInputService::FrameUpdate()
{
    return 0;
}

int CInputService::Cleanup()
{
    return 0;
}

bool CInputService::EventHook(void* event)
{
    auto* sdlEvent = static_cast<SDL_Event*>(event);

    switch (sdlEvent->type)
    {
        case SDL_MOUSEMOTION:
            //printf("[InputService] MouseMotion %d %d\n", sdlEvent->motion.x, sdlEvent->motion.y);
            break;
        case SDL_MOUSEBUTTONDOWN:
            //printf("[InputService] MouseDown %d\n", sdlEvent->button.button);
            break;
        case SDL_MOUSEBUTTONUP:
            //printf("[InputService] MouseUp %d\n", sdlEvent->button.button);
            break;
        case SDL_MOUSEWHEEL:
            //printf("[InputService] MouseWheel %d\n", sdlEvent->wheel.direction);
            break;
        default:
            break;
    }

	return false;
}

}