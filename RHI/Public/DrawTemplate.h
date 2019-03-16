#pragma once
#include "PipelineStateDesc.h"
#include "ShaderModule.h"
#include <vector>

namespace RHI
{

class CDrawTemplate
{
public:
    const std::vector<CViewportDesc>& GetViewports() const { return Viewports; }
    void SetViewports(std::vector<CViewportDesc> value) { Viewports = std::move(value); }

    const std::vector<CRectDesc>& GetScissors() const { return Scissors; }
    void SetScissors(std::vector<CRectDesc> value) { Scissors = std::move(value); }

    const CRasterizerDesc& GetRasterizerDesc() const { return Rasterizer; }
    void SetRasterizerDesc(CRasterizerDesc value) { Rasterizer = value; }
    
    const CDepthStencilDesc& GetDepthStencilDesc() const { return DepthStencil; }
    void SetDepthStencilDesc(CDepthStencilDesc value) { DepthStencil = value; }
    
    const CBlendDesc& GetBlendDesc() const { return Blend; }
    void SetBlendDesc(CBlendDesc value) { Blend = value; }

    const CVertexShaderInputBinding& GetVertexInputBinding() const { return VertexShaderInputBinding; }
    CVertexShaderInputBinding& GetVertexInputBinding() { return VertexShaderInputBinding; }

    sp<CShaderModule> GetVertexShader() const { return VertexShader; }
    void SetVertexShader(CShaderModule* value) { VertexShader = value; }

    sp<CShaderModule> GetPixelShader() const { return PixelShader; }
    void SetPixelShader(CShaderModule* value) { PixelShader = value; }

    const sp<CBuffer>& GetIndexBuffer() const { return IndexBuffer; }
    void SetIndexBuffer(CBuffer* value) { IndexBuffer = value; }

    const CPipelineArguments& GetPipelineArguments() const { return PipelineArgs; }
    CPipelineArguments& GetPipelineArguments() { return PipelineArgs; }

    bool IsIndexed = false;
    uint32_t ElementCount = 0;
    uint32_t InstanceCount = 0; //Zero means not instanced
    uint32_t VertexOffset = 0;
    uint32_t IndexOffset = 0;
    uint32_t InstanceOffset = 0;

private:
    std::vector<CViewportDesc> Viewports;
    std::vector<CRectDesc> Scissors;
    CRasterizerDesc Rasterizer;
    CDepthStencilDesc DepthStencil;
    CBlendDesc Blend;

    CVertexShaderInputBinding VertexShaderInputBinding;
    sp<CBuffer> IndexBuffer;
    CPipelineArguments PipelineArgs;

    sp<CShaderModule> VertexShader;
    sp<CShaderModule> PixelShader;
};

} /* namespace RHI */