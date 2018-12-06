#include "SDLService.h"

#include <SDL.h>
#include <glad/glad.h>

static bool bIsOpenGLOff = false;

namespace Nome
{

void CSDLService::SetOpenGLOff()
{
	bIsOpenGLOff = true;
}

int CSDLService::Setup()
{
	// Setup SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
	{
		printf("Error: %s\n", SDL_GetError());
		return -1;
	}

	if (bIsOpenGLOff)
	{
		Window = SDL_CreateWindow("Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_RESIZABLE);
		RenderContext = new CRenderContext(Window);
		return RenderContext->Init();
	}

	// Decide GL+GLSL versions
#if __APPLE__
	// GL 3.2 Core + GLSL 150
	GLSLVersion = "#version 150";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
	GLSLVersion = "#version 150";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#endif

	// Create window with graphics context
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_DisplayMode current;
	SDL_GetCurrentDisplayMode(0, &current);
	Window = SDL_CreateWindow("Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	GLContext = SDL_GL_CreateContext(Window);
	SDL_GL_SetSwapInterval(0);

	// Workaround Mojave black screen issue
	int width, height;
	SDL_GetWindowSize(Window, &width, &height);
	SDL_PumpEvents();
	SDL_SetWindowSize(Window, width, height);

	// Initialize OpenGL loader
	bool err = gladLoadGL() == 0;
	if (err)
	{
		fprintf(stderr, "Failed to initialize OpenGL loader!\n");
		return 1;
	}
    return 0;
}

int CSDLService::FrameUpdate()
{
    return 0;
}

int CSDLService::Cleanup()
{
	if (!bIsOpenGLOff)
		SDL_GL_DeleteContext(GLContext);
	else
	{
		RenderContext->Cleanup();
		delete RenderContext; RenderContext = nullptr;
	}
	SDL_DestroyWindow(Window);
	SDL_Quit();
    return 0;
}

bool CSDLService::EventLoopOnce()
{
	bool done = false;

	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT)
			done = true;
		if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(Window))
			done = true;

		//Handle window resize
		if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED && event.window.windowID == SDL_GetWindowID(Window))
		{
			int width;
			int height;
			SDL_GetWindowSize(Window, &width, &height);
			RenderContext->ResizeBackbuffer(width, height);
		}

		//Only pass down events belonging to this window
		if (event.window.windowID == SDL_GetWindowID(Window))
			RunEventHook(&event);
	}

	RunFrameUpdate();

	RunRender();
	RunRenderPhase2();
	if (!bIsOpenGLOff)
		SDL_GL_SwapWindow(Window);
	else
		RenderContext->Present();

	return done;
}

}