#include "RenderContext.h"
#include "CGLThread.h"
#include "ShaderManager.h"
#include <glad/glad.h>

static thread_local CRenderContext* CurrentRenderContext;

CRenderContext& CRenderContext::Current() { return *CurrentRenderContext; }

void CRenderContext::MakeCurrent() { CurrentRenderContext = this; }

CRenderContext::CRenderContext(bool singleThreaded)
{
    GLThread = std::make_shared<CGLThread>([=] { return true; }, [=] { return true; });
    MakeCurrent();
    if (singleThreaded)
        GLThread->SetSingleThreaded(true);
    else
        GLThread->Run();

    ShaderManager = std::make_unique<CShaderManager>();
}

#ifdef CONFIG_SDL_INTEGRATION
CRenderContext::CRenderContext(SDL_Window* window, SDL_GLContext glContext)
{
    GLThread = std::make_shared<CGLThread>([=] { return SDL_GL_MakeCurrent(window, glContext); },
                                           [=] { return SDL_GL_SwapWindow(window); });
    MakeCurrent();
    GLThread->Run();
}
#endif

CRenderContext::~CRenderContext() = default;

void CRenderContext::LoadGL(glloadproc_t proc) { gladLoadGLLoader(proc); }
