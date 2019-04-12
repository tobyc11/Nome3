#include <PresentationSurfaceDesc.h>
#include <RHIImGuiBackend.h>
#include <RHIInstance.h>
#include <SDL.h>
#include <SDL_syswm.h>
#include <Windows.h>
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

    auto depthImage = device->CreateImage2D(EFormat::D24_UNORM_S8_UINT,
                                            EImageUsageFlags::DepthStencil, width, height);
    CImageViewDesc depthViewDesc;
    depthViewDesc.Format = EFormat::D24_UNORM_S8_UINT;
    depthViewDesc.Type = EImageViewType::View2D;
    depthViewDesc.Range.Set(0, 1, 0, 1);
    auto depthView = device->CreateImageView(depthViewDesc, depthImage);

    CRenderPassDesc rpDesc;
    rpDesc.Attachments.emplace_back(fbView, CAttachmentLoadOp::Clear, CAttachmentStoreOp::Store);
    rpDesc.Attachments.emplace_back(depthView, CAttachmentLoadOp::Clear,
                                    CAttachmentStoreOp::DontCare);
    rpDesc.Subpasses.resize(1);
    rpDesc.Subpasses[0].AddColorAttachment(0);
    rpDesc.Subpasses[0].SetDepthStencilAttachment(1);
    swapChain->GetSize(rpDesc.Width, rpDesc.Height);
    rpDesc.Layers = 1;
    return device->CreateRenderPass(rpDesc);
}

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

    // Setup ImGui
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForVulkan(window);
    CRHIImGuiBackend::Init(device, screenPass);

    CPipeline::Ref pso;
    {
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
        pipelineDesc.RenderPass = screenPass;
        pso = device->CreatePipeline(pipelineDesc);
    }

    auto ubo = device->CreateBuffer(16, EBufferUsageFlags::ConstantBuffer);
    float* color = static_cast<float*>(ubo->Map(0, 16));
    color[0] = 1.0f;
    color[1] = 1.0f;
    color[2] = 1.0f;
    color[3] = 1.0f;
    ubo->Unmap();

    int x, y, comp;
    auto* checker512Data = stbi_load(APP_SOURCE_DIR "/checker512.png", &x, &y, &comp, 4);
    auto checker512 = device->CreateImage2D(EFormat::R8G8B8A8_UNORM, EImageUsageFlags::Sampled, 512,
                                            512, 1, 1, 1, checker512Data);
    stbi_image_free(checker512Data);
    CImageViewDesc checkerViewDesc;
    checkerViewDesc.Format = EFormat::R8G8B8A8_UNORM;
    checkerViewDesc.Type = EImageViewType::View2D;
    checkerViewDesc.Range.Set(0, 1, 0, 1);
    auto checkerView = device->CreateImageView(checkerViewDesc, checker512);

    CSamplerDesc samplerDesc;
    auto sampler = device->CreateSampler(samplerDesc);

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

        ctx->BeginRenderPass(screenPass,
                             { CClearValue(0.0f, 1.0f, 0.0f, 0.0f), CClearValue(1.0f, 0) });
        ctx->BindPipeline(pso);
        ctx->BindBuffer(ubo, 0, 16, 0, 1, 0);
        ctx->BindSampler(sampler, 1, 0, 0);
        ctx->BindImageView(checkerView, 1, 1, 0);
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
