#include "RenderService.h"

#include <glad/glad.h>

namespace Nome
{

int CRenderService::Setup()
{
    return 0;
}

void CRenderService::Render()
{
    //Example pipeline setup
    //VS

    //Rasterizer
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);

    //FS

    //FB
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_BLEND);
}

}
