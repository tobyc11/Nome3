#include "renderdoc_app.h"
#include <PresentationSurfaceDesc.h>
#include <RHIInstance.h>
#include <SDL.h>
#include <SDL_syswm.h>
#include <Windows.h>
#include <fstream>

using namespace RHI;

CShaderModule::Ref LoadSPIRV(CDevice::Ref device, const std::string& path)
{
    std::ifstream file(path.c_str(), std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (file.read(buffer.data(), size))
        return device->CreateShaderModule(buffer.size(), buffer.data());
    return {};
}

int main(int argc, char* argv[])
{
    auto device = CInstance::Get().CreateDevice(EDeviceCreateHints::Discrete);
    SDL_Window* window;
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("RHI Triangle Demo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              640, 480, 0);
    if (window == NULL)
    {
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(window, &wmInfo);

    CPresentationSurfaceDesc surfaceDesc;
    surfaceDesc.Type = EPresentationSurfaceDescType::Win32;
    surfaceDesc.Win32.Instance = wmInfo.info.win.hinstance;
    surfaceDesc.Win32.Window = wmInfo.info.win.window;
    auto swapChain = device->CreateSwapChain(surfaceDesc, EFormat::B8G8R8A8_UNORM);

    auto fbImage = swapChain->GetImage();

    CImageViewDesc fbViewDesc;
    fbViewDesc.Format = EFormat::B8G8R8A8_UNORM;
    fbViewDesc.Range.BaseMipLevel = 0;
    fbViewDesc.Range.LevelCount = 1;
    fbViewDesc.Range.BaseArrayLayer = 0;
    fbViewDesc.Range.LayerCount = 1;
    auto fbView = device->CreateImageView(fbViewDesc, fbImage);

    CRenderPassDesc rpDesc;
    rpDesc.Attachments.emplace_back(fbView, CAttachmentLoadOp::Clear, CAttachmentStoreOp::Store);
    rpDesc.Subpasses.resize(1);
    rpDesc.Subpasses[0].AddColorAttachment(0);
    swapChain->GetSize(rpDesc.Width, rpDesc.Height);
    rpDesc.Layers = 1;
    auto screenPass = device->CreateRenderPass(rpDesc);

    CPipelineDesc pipelineDesc;
    CRasterizerDesc rastDesc;
    CDepthStencilDesc depthStencilDesc;
    CBlendDesc blendDesc;
    rastDesc.CullMode = ECullModeFlags::None;
    depthStencilDesc.DepthTestEnable = false;
    depthStencilDesc.DepthWriteEnable = false;
    pipelineDesc.VS = LoadSPIRV(device, APP_SOURCE_DIR "/Shader/Demo1.vert.spv");
    pipelineDesc.PS = LoadSPIRV(device, APP_SOURCE_DIR "/Shader/Demo1.frag.spv");
    pipelineDesc.RasterizerState = &rastDesc;
    pipelineDesc.DepthStencilState = &depthStencilDesc;
    pipelineDesc.BlendState = &blendDesc;
    pipelineDesc.RenderPass = screenPass;
    auto pso = device->CreatePipeline(pipelineDesc);

    auto ctx = device->GetImmediateContext();
    bool done = false;
    while (!done)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                done = true;
        }

		swapChain->AcquireNextImage();

        ctx->BeginRenderPass(screenPass, { CClearValue(0.0f, 1.0f, 0.0f, 0.0f) });
        ctx->BindPipeline(pso.get());
        ctx->Draw(3, 1, 0, 0);
        ctx->EndRenderPass();

		CSwapChainPresentInfo info;
        info.SrcImage = fbView;
        swapChain->Present(info);
    }

	ctx->Flush(true);
    SDL_DestroyWindow(window);
    SDL_Quit();
    SDL_Delay(1000);
    return 0;
}
