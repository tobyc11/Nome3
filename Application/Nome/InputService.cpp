#include "InputService.h"

#include <SDL.h>

namespace Nome
{

void CInputService::AddMouseListener(IMouseListener* listener)
{
	MouseListeners.push_back(listener);
}

void CInputService::RemoveMouseListener(IMouseListener* listener)
{
	for (auto iter = MouseListeners.begin(); iter != MouseListeners.end(); ++iter)
	{
		if (*iter == listener)
		{
			MouseListeners.erase(iter);
			break;
		}
	}
}

int CInputService::Setup()
{
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

	if (bFirstFrame)
	{
		SDL_GetMouseState(&MouseState.x, &MouseState.y);
		MouseState.dx = 0;
		MouseState.dy = 0;
		bFirstFrame = false;
	}

    switch (sdlEvent->type)
    {
        case SDL_MOUSEMOTION:
            //printf("[InputService] MouseMotion %d %d\n", sdlEvent->motion.x, sdlEvent->motion.y);
			MouseState.dx = sdlEvent->motion.x - MouseState.x;
			MouseState.dy = sdlEvent->motion.y - MouseState.y;
			MouseState.x = sdlEvent->motion.x;
			MouseState.y = sdlEvent->motion.y;
			for (auto* listener : MouseListeners)
			{
				listener->MouseMoved(MouseState);
			}
            break;
        case SDL_MOUSEBUTTONDOWN:
            //printf("[InputService] MouseDown %d\n", sdlEvent->button.button);
			for (auto* listener : MouseListeners)
			{
				listener->MouseButtonPressed(sdlEvent->button.button);
			}
            break;
        case SDL_MOUSEBUTTONUP:
            //printf("[InputService] MouseUp %d\n", sdlEvent->button.button);
			for (auto* listener : MouseListeners)
			{
				listener->MouseButtonReleased(sdlEvent->button.button);
			}
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