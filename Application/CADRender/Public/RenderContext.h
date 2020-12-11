#pragma once
#include <cassert>
#include <memory>
#ifdef CONFIG_SDL_INTEGRATION
#include <SDL2/SDL.h>
#endif

typedef void* (*glloadproc_t)(const char* name);

class CGLThread;
class CShaderManager;

class CRenderContext
{
public:
    static CRenderContext& Current();
    void MakeCurrent();

    // Creates an offscreen render context
    explicit CRenderContext(bool singleThreaded = false);
#ifdef CONFIG_SDL_INTEGRATION
    // Creates a render context from a SDL GLContext, afterwards, the GLContext
    // is owned by render context. Please do not make it current on any other
    // thread.
    CRenderContext(SDL_Window* window, SDL_GLContext glContext);
#endif
    ~CRenderContext();

    CRenderContext(const CRenderContext&) = delete;
    CRenderContext& operator=(const CRenderContext&) = delete;

    CGLThread& GetDevice() { return *GLThread; }
    CShaderManager& GetShaderManager() { return *ShaderManager; }
    static void LoadGL(glloadproc_t proc);

private:
    std::shared_ptr<CGLThread> GLThread;
    std::unique_ptr<CShaderManager> ShaderManager;
};

class CRenderContextChild
{
public:
    CRenderContextChild()
    {
        ParentCtx = &CRenderContext::Current();
        assert(ParentCtx);
    }

    void AssertEqCtx(CRenderContext* other) const { assert(ParentCtx == other); }

protected:
    CRenderContext* ParentCtx;
};
