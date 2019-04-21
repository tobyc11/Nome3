#include <PresentationSurfaceDesc.h>
#include <RHIImGuiBackend.h>
#include <RHIInstance.h>
#include <SDL.h>
#include <SDL_syswm.h>
#include <Windows.h>
#include <chrono>
#include <fstream>

#include "imgui.h"
#include "imgui_impl_sdl.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace RHI;

static CShaderModule::Ref LoadSPIRV(CDevice::Ref device, const std::string& path)
{
    std::ifstream file(path.c_str(), std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (file.read(buffer.data(), size))
        return device->CreateShaderModule(buffer.size(), buffer.data());
    return {};
}

static CRenderPass::Ref CreateScreenPass(CDevice::Ref device, CSwapChain::Ref swapChain)
{
    auto fbImage = swapChain->GetImage();
    uint32_t width, height;
    swapChain->GetSize(width, height);

    CImageViewDesc fbViewDesc;
    fbViewDesc.Format = EFormat::R8G8B8A8_UNORM;
    fbViewDesc.Type = EImageViewType::View2D;
    fbViewDesc.Range.Set(0, 1, 0, 1);
    auto fbView = device->CreateImageView(fbViewDesc, fbImage);

    CRenderPassDesc rpDesc;
    rpDesc.AddAttachment(fbView, EAttachmentLoadOp::DontCare, EAttachmentStoreOp::Store);
    rpDesc.NextSubpass().AddColorAttachment(0);
    swapChain->GetSize(rpDesc.Width, rpDesc.Height);
    rpDesc.Layers = 1;
    return device->CreateRenderPass(rpDesc);
}

struct CMainPass
{
    CMainPass(CDevice::Ref device, uint32_t width, uint32_t height)
    {
        // Setup infrequently changed constants
        FragConstants = device->CreateBuffer(16, EBufferUsageFlags::ConstantBuffer);
        float* color = static_cast<float*>(FragConstants->Map(0, 16));
        color[0] = 1.0f;
        color[1] = 1.0f;
        color[2] = 1.0f;
        color[3] = 1.0f;
        FragConstants->Unmap();

        // Checker texture and its sampler
        int x, y, comp;
        auto* checker512Data = stbi_load(APP_SOURCE_DIR "/checker512.png", &x, &y, &comp, 4);
        auto checker512 = device->CreateImage2D(EFormat::R8G8B8A8_UNORM, EImageUsageFlags::Sampled,
                                                512, 512, 1, 1, 1, checker512Data);
        stbi_image_free(checker512Data);
        CImageViewDesc checkerViewDesc;
        checkerViewDesc.Format = EFormat::R8G8B8A8_UNORM;
        checkerViewDesc.Type = EImageViewType::View2D;
        checkerViewDesc.Range.Set(0, 1, 0, 1);
        CheckerView = device->CreateImageView(checkerViewDesc, checker512);

        CSamplerDesc samplerDesc;
        Sampler = device->CreateSampler(samplerDesc);

        // Render target and render pass
        ColorTarget = device->CreateImage2D(
            EFormat::R8G8B8A8_UNORM, EImageUsageFlags::RenderTarget | EImageUsageFlags::Sampled,
            width, height);
        CImageViewDesc colorViewDesc;
        colorViewDesc.Format = EFormat::R8G8B8A8_UNORM;
        colorViewDesc.Type = EImageViewType::View2D;
        colorViewDesc.Range.Set(0, 1, 0, 1);
        ColorTargetView = device->CreateImageView(colorViewDesc, ColorTarget);

        auto depthImage = device->CreateImage2D(EFormat::D24_UNORM_S8_UINT,
                                                EImageUsageFlags::DepthStencil, width, height);
        CImageViewDesc depthViewDesc;
        depthViewDesc.Format = EFormat::D24_UNORM_S8_UINT;
        depthViewDesc.Type = EImageViewType::View2D;
        depthViewDesc.Range.Set(0, 1, 0, 1);
        auto depthView = device->CreateImageView(depthViewDesc, depthImage);

        CRenderPassDesc passDesc;
        passDesc.AddAttachment(ColorTargetView, EAttachmentLoadOp::Clear,
                               EAttachmentStoreOp::Store);
        passDesc.AddAttachment(depthView, EAttachmentLoadOp::Clear, EAttachmentStoreOp::DontCare);
        passDesc.NextSubpass().AddColorAttachment(0).SetDepthStencilAttachment(1);
        passDesc.SetExtent(width, height);
        RenderPass = device->CreateRenderPass(passDesc);

        // Pipieline state object
        CPipelineDesc pipelineDesc;
        CRasterizerDesc rastDesc;
        CDepthStencilDesc depthStencilDesc;
        CBlendDesc blendDesc;
        rastDesc.CullMode = ECullModeFlags::None;
        pipelineDesc.VS = LoadSPIRV(device, APP_SOURCE_DIR "/Shader/Demo1.vert.spv");
        pipelineDesc.PS = LoadSPIRV(device, APP_SOURCE_DIR "/Shader/Demo2.frag.spv");
        pipelineDesc.RasterizerState = &rastDesc;
        pipelineDesc.DepthStencilState = &depthStencilDesc;
        pipelineDesc.BlendState = &blendDesc;
        pipelineDesc.RenderPass = RenderPass;
        Pipeline = device->CreatePipeline(pipelineDesc);
    }

    void ExecutePass(IRenderContext* ctx, float animationT)
    {
        ctx->BeginRenderPass(*RenderPass,
                             { CClearValue(0.0f, 1.0f, 0.0f, 0.0f), CClearValue(1.0f, 0) });
        ctx->BindPipeline(*Pipeline);
        ctx->BindBuffer(*FragConstants, 0, 16, 0, 2, 0);
        ctx->BindSampler(*Sampler, 0, 0, 0);
        ctx->BindImageView(*CheckerView, 0, 1, 0);

        for (int j = 0; j < 2; j++)
            for (int i = 0; i < 2; i++)
            {
                std::array<float, 8> perObjectData;
                perObjectData[0] = cosf(i * animationT * 0.01f) * (j + 1) / 16.0f;
                perObjectData[1] = sinf(i * animationT * 0.01f) * (j + 1) / 16.0f;
                perObjectData[4] = 0.15f * (sinf(i * animationT * 0.1f) + 0.5f);
                perObjectData[5] = 0.15f * (cosf(i * animationT * 0.1f) + 0.5f);
                ctx->BindConstants(perObjectData.data(), 32, 1, 0, 0);
                ctx->Draw(3, 1, 0, 0);
            }
        ctx->EndRenderPass();
    }

    CImage::Ref ColorTarget;
    CImageView::Ref ColorTargetView;
    CRenderPass::Ref RenderPass;

    CPipeline::Ref Pipeline;
    CBuffer::Ref FragConstants;
    CImageView::Ref CheckerView;
    CSampler::Ref Sampler;
};

int main(int argc, char* argv[])
{
    auto device = CInstance::Get().CreateDevice(EDeviceCreateHints::Integrated);
    SDL_Window* window;
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("RHI Triangle Demo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              640, 480, SDL_WINDOW_RESIZABLE);
    if (window == nullptr)
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
    auto swapChain = device->CreateSwapChain(surfaceDesc, EFormat::R8G8B8A8_UNORM);

    auto screenPass = CreateScreenPass(device, swapChain);

    CPipeline::Ref blitPipeline;
    {
        CPipelineDesc blitterDesc;
        CRasterizerDesc blitterRast;
        blitterRast.CullMode = ECullModeFlags::None;
        CDepthStencilDesc blitterDepth;
        blitterDepth.DepthTestEnable = false;
        blitterDepth.DepthWriteEnable = false;
        CBlendDesc blitterBlend;
        blitterDesc.VS = LoadSPIRV(device, APP_SOURCE_DIR "/Shader/Quad.vert.spv");
        blitterDesc.PS = LoadSPIRV(device, APP_SOURCE_DIR "/Shader/Blit.frag.spv");
        blitterDesc.RenderPass = screenPass;
        blitterDesc.RasterizerState = &blitterRast;
        blitterDesc.DepthStencilState = &blitterDepth;
        blitterDesc.BlendState = &blitterBlend;
        blitPipeline = device->CreatePipeline(blitterDesc);
    }

    CMainPass mainPass { device, 320, 240 };

    // Setup ImGui
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForVulkan(window);
    CRHIImGuiBackend::Init(device, screenPass);

    auto timeStart = std::chrono::steady_clock::now();
    auto ctx = device->GetImmediateContext();
    bool done = false;
    while (!done)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED)
            {
                screenPass.reset();
                swapChain->Resize(UINT32_MAX, UINT32_MAX);
                screenPass = CreateScreenPass(device, swapChain);
            }
        }

        CRHIImGuiBackend::NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();

        bool swapOk = swapChain->AcquireNextImage();
        if (!swapOk)
        {
            screenPass.reset();
            swapChain->Resize(UINT32_MAX, UINT32_MAX);
            screenPass = CreateScreenPass(device, swapChain);
            swapChain->AcquireNextImage();
        }

        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<float> animT(now - timeStart);
        mainPass.ExecutePass(ctx.get(), animT.count());

        ctx->BeginRenderPass(*screenPass, { CClearValue(0.0f, 1.0f, 0.0f, 0.0f) });

        ctx->BindPipeline(*blitPipeline);
        ctx->BindSampler(*mainPass.Sampler, 0, 0, 0);
        ctx->BindImageView(*mainPass.ColorTargetView, 0, 1, 0);
        ctx->Draw(3, 1, 0, 0);

        ImGui::Render();
        CRHIImGuiBackend::RenderDrawData(ImGui::GetDrawData(), ctx);

        ctx->EndRenderPass();

        CSwapChainPresentInfo info;
        info.SrcImage = nullptr;
        swapChain->Present(info);
    }
    ctx->Flush(true);
    CRHIImGuiBackend::Shutdown();
    SDL_DestroyWindow(window);
    SDL_Quit();
    SDL_Delay(1000);
    return 0;
}
