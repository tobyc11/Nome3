#include "renderdoc_app.h"
#include <RHIInstance.h>
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

int main()
{
    HMODULE renderDoc = GetModuleHandle("renderdoc.dll");
    RENDERDOC_API_1_2_0* rdocApi = nullptr;
    if (renderDoc)
    {
        printf("Found render doc\n");
        pRENDERDOC_GetAPI rdGetAPI =
            (pRENDERDOC_GetAPI)GetProcAddress(renderDoc, "RENDERDOC_GetAPI");
        int ret = rdGetAPI(eRENDERDOC_API_Version_1_2_0, (void**)&rdocApi);
        assert(ret == 1);
    }
    auto& rhi = CInstance::Get();
    auto device = rhi.CreateDevice(EDeviceCreateHints::NoHint);

    auto fbImage =
        device->CreateImage2D(EFormat::R8G8B8A8_UNORM, EImageUsageFlags::RenderTarget, 1024, 1024);

    auto readBackImage =
        device->CreateImage2D(EFormat::R8G8B8A8_UNORM, EImageUsageFlags::Staging, 1024, 1024);

    CImageViewDesc fbViewDesc;
    fbViewDesc.Format = EFormat::R8G8B8A8_UNORM;
    fbViewDesc.Range.BaseMipLevel = 0;
    fbViewDesc.Range.LevelCount = 1;
    fbViewDesc.Range.BaseArrayLayer = 0;
    fbViewDesc.Range.LayerCount = 1;
    auto fbView = device->CreateImageView(fbViewDesc, fbImage);

    CRenderPassDesc rpDesc;
    rpDesc.Attachments.emplace_back(fbView, CAttachmentLoadOp::Clear, CAttachmentStoreOp::Store);
    rpDesc.Subpasses.resize(1);
    rpDesc.Subpasses[0].AddColorAttachment(0);
    rpDesc.Width = 1024;
    rpDesc.Height = 1024;
    rpDesc.Layers = 1;
    auto renderPass = device->CreateRenderPass(rpDesc);

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
    pipelineDesc.RenderPass = renderPass;
    auto pso = device->CreatePipeline(pipelineDesc);

    if (rdocApi)
        rdocApi->StartFrameCapture(nullptr, nullptr);

    auto ctx = device->GetImmediateContext();
    ctx->BeginRenderPass(renderPass, { CClearValue(0.0f, 1.0f, 0.0f, 0.0f) });
    ctx->BindPipeline(pso.get());
    ctx->Draw(3, 1, 0, 0);
    ctx->EndRenderPass();

    CImageCopy copy;
    copy.SrcSubresource.MipLevel = 0;
    copy.SrcSubresource.BaseArrayLayer = 0;
    copy.SrcSubresource.LayerCount = 1;
    copy.SrcOffset.Set(0, 0, 0);
    copy.DstSubresource = copy.SrcSubresource;
    copy.DstOffset = copy.SrcOffset;
    copy.Extent.Set(1024, 1024, 1);
    ctx->CopyImage(fbImage.get(), readBackImage.get(), { copy });

    ctx->Flush(true);
    if (rdocApi)
        rdocApi->EndFrameCapture(nullptr, nullptr);
    Sleep(1000);
    return 0;
}
