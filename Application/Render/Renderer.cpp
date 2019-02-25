#include "Renderer.h"
#include "GraphicsDevice.h"
#include "Geometry.h"
#include "Material.h"
#include "Viewport.h"
#include "ShaderCommon.h"
#include "UniformBuffer.h"
#include "ResourceMgr.h"
#include "StateBlocks.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Nome
{

static CRenderer StaticRenderer;
CRenderer* GRenderer = &StaticRenderer;

//TODO: put this somewhere else
struct CRendererPrivData
{
    ComPtr<ID3D11ShaderResourceView> DotSRV;
};

CRenderer::CRenderer()
{
    GD = new CGraphicsDevice();
    auto* dev = GetGD()->GetDevice();

    Pd = new CRendererPrivData();

    //Load dot texture
    int x, y, n;
    unsigned char* data = stbi_load(CResourceMgr::Get().Find("Textures/dot16.png").c_str(), &x, &y, &n, 4);
    CD3D11_TEXTURE2D_DESC desc{ DXGI_FORMAT_R8G8B8A8_UNORM, (uint32_t)x, (uint32_t)y, 1, 1 };
    D3D11_SUBRESOURCE_DATA initialData;
    initialData.pSysMem = data;
    initialData.SysMemPitch = x * 4;
    initialData.SysMemSlicePitch = 0;
    ComPtr<ID3D11Texture2D> texture;
    dev->CreateTexture2D(&desc, &initialData, texture.GetAddressOf());
    CD3D11_SHADER_RESOURCE_VIEW_DESC sdesc{ texture.Get(), D3D11_SRV_DIMENSION_TEXTURE2D };
    dev->CreateShaderResourceView(texture.Get(), &sdesc, Pd->DotSRV.GetAddressOf());
    stbi_image_free(data);
}

CRenderer::~CRenderer()
{
    delete Pd;
    delete GD;
}

void CRenderer::BeginView(const CRenderViewInfo& viewInfo)
{
    CViewData viewData;
    CRenderViewInfo& viewDataInfo = viewData;
    viewDataInfo = viewInfo;
    Views.push_back(viewData);
}

void CRenderer::EndView()
{
}

void CRenderer::Draw(const tc::Matrix4& modelMat, CGeometry* geometry, CMaterial* material)
{
    CViewData& currView = Views[Views.size() - 1];
    CViewData::CObjectData objData;
    objData.Geom = geometry;
    objData.Material = material;
    objData.ModelMat = modelMat;
    currView.DrawListBasic.insert(objData);
}

class CBasicShader : public CShaderCommon
{
public:
    CBasicShader()
    {
        //Read the shader template from disk
        std::ifstream ifs(CResourceMgr::Get().Find("Basic.hlsl"));
        std::string str;

        ifs.seekg(0, std::ios::end);
        str.reserve(ifs.tellg());
        ifs.seekg(0, std::ios::beg);

        str.assign((std::istreambuf_iterator<char>(ifs)),
                   std::istreambuf_iterator<char>());

        auto* dev = GRenderer->GetGD()->GetDevice();
        {
            //Compile vertex shader
            ID3DBlob* vsCode;
            ID3DBlob* errorBlob;
            D3DCompile(str.c_str(), str.size(), "Basic.hlsl", nullptr, nullptr, "VSmain", "vs_5_0", 0, 0, &vsCode, &errorBlob);
            if (errorBlob)
            {
                std::cout << "[VS] Shader compilation error:" << std::endl;
                std::cout << reinterpret_cast<const char*>(errorBlob->GetBufferPointer()) << std::endl;
                errorBlob->Release();
                return;
            }

            //Create the vertex shader
            dev->CreateVertexShader(vsCode->GetBufferPointer(), vsCode->GetBufferSize(), nullptr, VS.GetAddressOf());

            //Create input layout
            D3D11_INPUT_ELEMENT_DESC desc[] = {
                { "ATTRIBUTE", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "ATTRIBUTE", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
            };
            dev->CreateInputLayout(desc, sizeof(desc) / sizeof(D3D11_INPUT_ELEMENT_DESC),
                vsCode->GetBufferPointer(), vsCode->GetBufferSize(), DefaultInputLayout.GetAddressOf());
        }

        {
            //Compile pixel shader
            ID3DBlob* byteCode;
            ID3DBlob* errorBlob;
            D3DCompile(str.c_str(), str.size(), "Basic.hlsl", nullptr, nullptr, "PSmain", "ps_5_0", 0, 0, &byteCode, &errorBlob);
            if (errorBlob)
            {
                std::cout << "[PS] Shader compilation error:" << std::endl;
                std::cout << reinterpret_cast<const char*>(errorBlob->GetBufferPointer()) << std::endl;
                errorBlob->Release();
                return;
            }

            //Create the pixel shader
            dev->CreatePixelShader(byteCode->GetBufferPointer(), byteCode->GetBufferSize(), nullptr, PS.GetAddressOf());
        }

        //Create constant buffer
        D3D11_BUFFER_DESC desc;
        desc.ByteWidth = sizeof(CBEverythingBuf);
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;
        desc.StructureByteStride = 0;
        dev->CreateBuffer(&desc, nullptr, CBEverything.GetAddressOf());
    }

    //TODO: abstract constant buffers into a generic class
    struct CBEverythingDef
    {
        tc::Matrix4 Model;
        tc::Matrix4 View;
        tc::Matrix4 Proj;
        tc::Vector4 Color;
    } CBEverythingBuf;

    CBEverythingDef& GetCBEverything() { return CBEverythingBuf; }
    void UpdateCBEverything(ID3D11DeviceContext* ctx)
    {
        D3D11_MAPPED_SUBRESOURCE cbMapped;
        ctx->Map(CBEverything.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &cbMapped);
        memcpy(cbMapped.pData, &CBEverythingBuf, sizeof(CBEverythingBuf));
        ctx->Unmap(CBEverything.Get(), 0);
    }

    void Bind(ID3D11DeviceContext* ctx, bool bindInputLayout = true)
    {
        ID3D11Buffer* buffersToBind[] = { CBEverything.Get() };
        ctx->VSSetShader(VS.Get(), nullptr, 0);
        ctx->VSSetConstantBuffers(0, 1, buffersToBind);
        ctx->PSSetShader(PS.Get(), nullptr, 0);
        ctx->PSSetConstantBuffers(0, 1, buffersToBind);

        if (bindInputLayout)
            ctx->IASetInputLayout(DefaultInputLayout.Get());
    }

private:
    ComPtr<ID3D11InputLayout> DefaultInputLayout;
    ComPtr<ID3D11Buffer> CBEverything;
};

class CWireShader : public CShaderCommon
{
public:
    CWireShader()
    {
        auto* dev = GRenderer->GetGD()->GetDevice();
        CompileFile(CResourceMgr::Get().Find("Wire.hlsl"), "VSmain", "vs_5_0");
        //Create input layout
        D3D11_INPUT_ELEMENT_DESC ilDesc[] = {
            { "ATTRIBUTE", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };
        dev->CreateInputLayout(ilDesc, sizeof(ilDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC),
            VSBytecode->GetBufferPointer(), VSBytecode->GetBufferSize(), DefaultInputLayout.GetAddressOf());
        dev->CreateVertexShader(VSBytecode->GetBufferPointer(), VSBytecode->GetBufferSize(), nullptr, VS.GetAddressOf());
        CompileFile(CResourceMgr::Get().Find("Wire.hlsl"), "GSmain", "gs_5_0");
        dev->CreateGeometryShader(GSBytecode->GetBufferPointer(), GSBytecode->GetBufferSize(), nullptr, GS.GetAddressOf());
        GSBytecode = nullptr;
        CompileFile(CResourceMgr::Get().Find("Wire.hlsl"), "PSmain", "ps_5_0");
        dev->CreatePixelShader(PSBytecode->GetBufferPointer(), PSBytecode->GetBufferSize(), nullptr, PS.GetAddressOf());
        PSBytecode = nullptr;

        //Create constant buffer
        D3D11_BUFFER_DESC desc;
        desc.ByteWidth = sizeof(CBEverythingBuf);
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;
        desc.StructureByteStride = 0;
        dev->CreateBuffer(&desc, nullptr, CBEverything.GetAddressOf());
    }

    //TODO: abstract constant buffers into a generic class
    struct CBEverythingDef
    {
        tc::Matrix4 Model;
        tc::Matrix4 View;
        tc::Matrix4 Proj;
        tc::Vector4 Color;
        float LineWidth;
        float Width;
        float Height;
        float Padding;
    } CBEverythingBuf;

    CBEverythingDef& GetCBEverything() { return CBEverythingBuf; }
    void UpdateCBEverything(ID3D11DeviceContext* ctx)
    {
        D3D11_MAPPED_SUBRESOURCE cbMapped;
        ctx->Map(CBEverything.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &cbMapped);
        memcpy(cbMapped.pData, &CBEverythingBuf, sizeof(CBEverythingBuf));
        ctx->Unmap(CBEverything.Get(), 0);
    }

    void Bind(ID3D11DeviceContext* ctx, bool bindInputLayout = true)
    {
        ID3D11Buffer* buffersToBind[] = { CBEverything.Get() };
        ctx->VSSetShader(VS.Get(), nullptr, 0);
        ctx->VSSetConstantBuffers(0, 1, buffersToBind);
        ctx->GSSetShader(GS.Get(), nullptr, 0);
        ctx->GSSetConstantBuffers(0, 1, buffersToBind);
        ctx->PSSetShader(PS.Get(), nullptr, 0);
        ctx->PSSetConstantBuffers(0, 1, buffersToBind);

        if (bindInputLayout)
            ctx->IASetInputLayout(DefaultInputLayout.Get());
    }

private:
    ComPtr<ID3D11InputLayout> DefaultInputLayout;
    ComPtr<ID3D11Buffer> CBEverything;
};

//Point rendering
BEGIN_UNIFORM_BUFFER(CBPerView)
    MEMBER(Matrix4, View)
    MEMBER(Matrix4, Proj)
    MEMBER(float, Width)
    MEMBER(float, Height)
END_UNIFORM_BUFFER()

BEGIN_UNIFORM_BUFFER(CBPerObject)
    MEMBER(Matrix4, Model)
END_UNIFORM_BUFFER()

BEGIN_UNIFORM_BUFFER(CBPoint)
    MEMBER(float, PointRadius)
    MEMBER(bool, bColorCodeByWorldPos)
END_UNIFORM_BUFFER()

class CPointShader : public CShaderCommon
{
public:
    CPointShader()
    {
        auto* dev = GRenderer->GetGD()->GetDevice();
        CompileFile(CResourceMgr::Get().Find("Point.hlsl"), "VSmain", "vs_5_0");
        //Create input layout
        D3D11_INPUT_ELEMENT_DESC ilDesc[] = {
            { "ATTRIBUTE", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "ATTRIBUTE", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };
        dev->CreateInputLayout(ilDesc, sizeof(ilDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC),
            VSBytecode->GetBufferPointer(), VSBytecode->GetBufferSize(), DefaultInputLayout.GetAddressOf());
        dev->CreateVertexShader(VSBytecode->GetBufferPointer(), VSBytecode->GetBufferSize(), nullptr, VS.GetAddressOf());
        CompileFile(CResourceMgr::Get().Find("Point.hlsl"), "GSmain", "gs_5_0");
        dev->CreateGeometryShader(GSBytecode->GetBufferPointer(), GSBytecode->GetBufferSize(), nullptr, GS.GetAddressOf());
        GSBytecode = nullptr;
        CompileFile(CResourceMgr::Get().Find("Point.hlsl"), "PSmain", "ps_5_0");
        dev->CreatePixelShader(PSBytecode->GetBufferPointer(), PSBytecode->GetBufferSize(), nullptr, PS.GetAddressOf());
        PSBytecode = nullptr;
    }

    void Bind(ID3D11DeviceContext* ctx, bool bindInputLayout = true)
    {
        ctx->VSSetShader(VS.Get(), nullptr, 0);
        ctx->GSSetShader(GS.Get(), nullptr, 0);
        ctx->PSSetShader(PS.Get(), nullptr, 0);

        if (bindInputLayout)
            ctx->IASetInputLayout(DefaultInputLayout.Get());
    }

    //TODO: move this somewhere else
    template <typename T>
    static TTypedUniformBufferRef<T> CreateUniformBuffer(T* data)
    {
        auto* dev = GRenderer->GetGD()->GetDevice();

        //Create constant buffer
        ComPtr<ID3D11Buffer> buffer;

        D3D11_BUFFER_DESC desc;
        desc.ByteWidth = sizeof(T) / 16 * 16 + 16; //TODO: verify this
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;
        desc.StructureByteStride = 0;
        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = data;
        dev->CreateBuffer(&desc, &initData, buffer.GetAddressOf());

        return { buffer };
    }

private:
    ComPtr<ID3D11InputLayout> DefaultInputLayout;
};

void CRenderer::Render()
{
    //Once upon a time, there was drawing policy and shader manager and such...
    //  But I am lazy.
    if (!BasicShader)
        BasicShader.reset(new CBasicShader());
    if (!WireShader)
        WireShader.reset(new CWireShader());
    if (!PointShader)
        PointShader.reset(new CPointShader());

    auto* ctx = GetGD()->GetImmediateContext();
    auto& cbBasic = BasicShader->GetCBEverything();
    auto& cbEverything = WireShader->GetCBEverything();
    for (const CViewData& view : Views)
    {
        cbBasic.View = view.ViewMat;
        cbBasic.Proj = view.ProjMat;
        cbEverything.View = view.ViewMat;
        cbEverything.Proj = view.ProjMat;
        view.Viewport->BindAndClear(ctx, view.ClearColor.Data());
        BasicShader->Bind(ctx);
        for (const CViewData::CObjectData& obj : view.DrawListBasic)
        {
            if (auto* geometry = dynamic_cast<CStaticMeshGeometry*>(obj.Geom))
            {
                cbBasic.Model = obj.ModelMat;
                cbBasic.Color = tc::Vector4(obj.Material->GetColor(), 1.0f);
                BasicShader->UpdateCBEverything(ctx);
                obj.Material->Bind(ctx);

                //Bind vertex buffers
                auto* posAttr = geometry->GetAttribute("POSITION");
                ID3D11Buffer* vertexBuffer = posAttr->Buffer->GetD3D11Buffer();
                ctx->IASetVertexBuffers(0, 1, &vertexBuffer, &posAttr->Stride, &posAttr->Offset);
                ctx->IASetPrimitiveTopology(geometry->GetPrimitiveTopology());
                if (TAutoPtr<CIndexBuffer> indexBuffer = geometry->GetIndexBuffer())
                {
                    indexBuffer->Bind(ctx);
                    ctx->DrawIndexed(geometry->GetElementCount(), 0, 0);
                }
                else
                {
                    ctx->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
                    ctx->Draw(geometry->GetElementCount(), 0);
                }
            }
        }
        WireShader->Bind(ctx);
        for (const CViewData::CObjectData& obj : view.DrawListBasic)
        {
            if (auto* geometry = dynamic_cast<CStaticMeshGeometry*>(obj.Geom))
            {
                if (geometry->GetPrimitiveTopology() == D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP)
                    continue; //Wireshader currently does not support line primitive types

                cbEverything.Model = obj.ModelMat;
                cbEverything.Color = tc::Vector4(obj.Material->GetColor(), 1.0f);
                cbEverything.LineWidth = view.LineWidth;
                cbEverything.Width = view.Viewport->GetWidth();
                cbEverything.Height = view.Viewport->GetHeight();
                WireShader->UpdateCBEverything(ctx);
                obj.Material->Bind(ctx);
                ctx->RSSetState(NoCullBiasedRasterizerState.Get());

                //Bind vertex buffers
                auto* posAttr = geometry->GetAttribute("POSITION");
                ID3D11Buffer* vertexBuffer = posAttr->Buffer->GetD3D11Buffer();
                ctx->IASetVertexBuffers(0, 1, &vertexBuffer, &posAttr->Stride, &posAttr->Offset);
                ctx->IASetPrimitiveTopology(geometry->GetPrimitiveTopology());
                if (TAutoPtr<CIndexBuffer> indexBuffer = geometry->GetIndexBuffer())
                {
                    indexBuffer->Bind(ctx);
                    ctx->DrawIndexed(geometry->GetElementCount(), 0, 0);
                }
                else
                {
                    ctx->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
                    ctx->Draw(geometry->GetElementCount(), 0);
                }
            }
        }

        //Render points
        PointShader->Bind(ctx);
        CBPerView cbPerView;
        cbPerView.View = view.ViewMat;
        cbPerView.Proj = view.ProjMat;
        cbPerView.Width = view.Viewport->GetWidth();
        cbPerView.Height = view.Viewport->GetHeight();
        auto cbPerViewRef = PointShader->CreateUniformBuffer(&cbPerView);
        CBPoint cbPoint;
        cbPoint.PointRadius = view.PointRadius;
        cbPoint.bColorCodeByWorldPos = true;
        auto cbPointRef = PointShader->CreateUniformBuffer(&cbPoint);
        for (const CViewData::CObjectData& obj : view.DrawListBasic)
        {
            if (auto* geometry = dynamic_cast<CStaticMeshGeometry*>(obj.Geom))
            {
                CBPerObject cbPerObject;
                cbPerObject.Model = obj.ModelMat;
                auto cbPerObjectRef = PointShader->CreateUniformBuffer(&cbPerObject);

                ID3D11Buffer* constBuffers[] = { cbPerViewRef.Get(), cbPerObjectRef.Get(), cbPointRef.Get() };
                ctx->VSSetConstantBuffers(0, 3, constBuffers);
                ctx->GSSetConstantBuffers(0, 3, constBuffers);
                ctx->PSSetConstantBuffers(0, 3, constBuffers);
                ID3D11SamplerState* sampler = DefaultSamplerState.Get();
                ctx->PSSetSamplers(0, 1, &sampler);
                ctx->PSSetShaderResources(0, 1, Pd->DotSRV.GetAddressOf());

                ctx->RSSetState(NoCullBiasedRasterizerState.Get());
                ctx->OMSetDepthStencilState(DefaultDepthStencilState.Get(), 0);
                ctx->OMSetBlendState(PointBlendState.Get(), nullptr, 0xffffffff);

                //Bind vertex buffers
                auto* posAttr = geometry->GetAttribute("POSITION");
                ID3D11Buffer* vertexBuffer = posAttr->Buffer->GetD3D11Buffer();
                ctx->IASetVertexBuffers(0, 1, &vertexBuffer, &posAttr->Stride, &posAttr->Offset);
                ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
                if (TAutoPtr<CIndexBuffer> indexBuffer = geometry->GetIndexBuffer())
                {
                    indexBuffer->Bind(ctx);
                    ctx->DrawIndexed(geometry->GetElementCount(), 0, 0);
                }
                else
                {
                    ctx->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
                    ctx->Draw(geometry->GetElementCount(), 0);
                }
            }
        }
    }
    ctx->GSSetShader(nullptr, nullptr, 0);
    Views.clear();
}

class CPointPickingShader : public CShaderCommon
{
public:
    CPointPickingShader()
    {
        auto* dev = GRenderer->GetGD()->GetDevice();
        CompileFile(CResourceMgr::Get().Find("PointPicking.hlsl"), "VSmain", "vs_5_0");
        //Create input layout
        D3D11_INPUT_ELEMENT_DESC ilDesc[] = {
            { "ATTRIBUTE", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "ATTRIBUTE", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "ATTRIBUTE", 2, DXGI_FORMAT_R32_UINT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };
        dev->CreateInputLayout(ilDesc, sizeof(ilDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC),
            VSBytecode->GetBufferPointer(), VSBytecode->GetBufferSize(), DefaultInputLayout.GetAddressOf());
        dev->CreateVertexShader(VSBytecode->GetBufferPointer(), VSBytecode->GetBufferSize(), nullptr, VS.GetAddressOf());
        CompileFile(CResourceMgr::Get().Find("PointPicking.hlsl"), "GSmain", "gs_5_0");
        dev->CreateGeometryShader(GSBytecode->GetBufferPointer(), GSBytecode->GetBufferSize(), nullptr, GS.GetAddressOf());
        GSBytecode = nullptr;
        CompileFile(CResourceMgr::Get().Find("PointPicking.hlsl"), "PSmain", "ps_5_0");
        dev->CreatePixelShader(PSBytecode->GetBufferPointer(), PSBytecode->GetBufferSize(), nullptr, PS.GetAddressOf());
        PSBytecode = nullptr;
    }

    void Bind(ID3D11DeviceContext* ctx, bool bindInputLayout = true)
    {
        ctx->VSSetShader(VS.Get(), nullptr, 0);
        ctx->GSSetShader(GS.Get(), nullptr, 0);
        ctx->PSSetShader(PS.Get(), nullptr, 0);

        if (bindInputLayout)
            ctx->IASetInputLayout(DefaultInputLayout.Get());
    }

private:
    ComPtr<ID3D11InputLayout> DefaultInputLayout;
};

uint32_t CRenderer::RenderPickingPointsGetId(const CRenderViewInfo& viewInfo, CVertexBuffer* pointBuffer, uint32_t x, uint32_t y)
{
    if (!PointPickingShader)
        PointPickingShader.reset(new CPointPickingShader());

    auto* dev = GetGD()->GetDevice();
    auto* ctx = GetGD()->GetImmediateContext();

    //Render to texture
    uint32_t RTWidth = viewInfo.Viewport->GetWidth();
    uint32_t RTHeight = viewInfo.Viewport->GetHeight();
    CD3D11_TEXTURE2D_DESC texDesc{ DXGI_FORMAT_R32_UINT, RTWidth, RTHeight, 1, 1,
        D3D11_BIND_RENDER_TARGET, D3D11_USAGE_DEFAULT, 0 };
    ComPtr<ID3D11Texture2D> RenderTexture;
    dev->CreateTexture2D(&texDesc, nullptr, RenderTexture.GetAddressOf());
    ComPtr<ID3D11RenderTargetView> RenderTarget;
    CD3D11_RENDER_TARGET_VIEW_DESC rtvDesc{ RenderTexture.Get(), D3D11_RTV_DIMENSION_TEXTURE2D, DXGI_FORMAT_UNKNOWN, 0 };
    dev->CreateRenderTargetView(RenderTexture.Get(), &rtvDesc, RenderTarget.GetAddressOf());

    ComPtr<ID3D11DepthStencilView> DepthBufferView;
    {
        ID3D11Texture2D* pDepthStencil = nullptr;
        D3D11_TEXTURE2D_DESC desc;
        desc.Width = RTWidth;
        desc.Height = RTHeight;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = 0;
        HRESULT hr = dev->CreateTexture2D(&desc, nullptr, &pDepthStencil);
        if (!SUCCEEDED(hr))
            throw std::runtime_error("Cannot create depth buffer!");

        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = 0;
        dev->CreateDepthStencilView(pDepthStencil, &dsvDesc, DepthBufferView.GetAddressOf());
        pDepthStencil->Release();
    }

    //Clearing
    D3D11_VIEWPORT desc;
    desc.TopLeftX = 0;
    desc.TopLeftY = 0;
    desc.Width = RTWidth;
    desc.Height = RTHeight;
    desc.MinDepth = 0.0f;
    desc.MaxDepth = 1.0f;
    ctx->RSSetViewports(1, &desc);
    ctx->OMSetRenderTargets(1, RenderTarget.GetAddressOf(), DepthBufferView.Get());
    ctx->ClearDepthStencilView(DepthBufferView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    const float defaultColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    ctx->ClearRenderTargetView(RenderTarget.Get(), defaultColor);

    //Render points
    PointPickingShader->Bind(ctx);
    CBPerView cbPerView;
    cbPerView.View = viewInfo.ViewMat;
    cbPerView.Proj = viewInfo.ProjMat;
    cbPerView.Width = RTWidth;
    cbPerView.Height = RTHeight;
    auto cbPerViewRef = PointShader->CreateUniformBuffer(&cbPerView);
    CBPoint cbPoint;
    cbPoint.PointRadius = viewInfo.PointRadius;
    cbPoint.bColorCodeByWorldPos = true;
    auto cbPointRef = PointShader->CreateUniformBuffer(&cbPoint);

    CBPerObject cbPerObject;
    cbPerObject.Model = Matrix4::IDENTITY;
    auto cbPerObjectRef = PointShader->CreateUniformBuffer(&cbPerObject);

    ID3D11Buffer* constBuffers[] = { cbPerViewRef.Get(), cbPerObjectRef.Get(), cbPointRef.Get() };
    ctx->VSSetConstantBuffers(0, 3, constBuffers);
    ctx->GSSetConstantBuffers(0, 3, constBuffers);
    ctx->PSSetConstantBuffers(0, 3, constBuffers);
    auto* sampler = DefaultSamplerState.Get();
    ctx->PSSetSamplers(0, 1, &sampler);
    ctx->PSSetShaderResources(0, 1, Pd->DotSRV.GetAddressOf());

    ctx->RSSetState(NoCullBiasedRasterizerState.Get());
    ctx->OMSetDepthStencilState(DefaultDepthStencilState.Get(), 0);
    ctx->OMSetBlendState(DefaultBlendState.Get(), nullptr, 0xffffffff);

    //Calc how many points
    auto* pointBufferD3d = pointBuffer->GetD3D11Buffer();
    D3D11_BUFFER_DESC bufDesc;
    pointBufferD3d->GetDesc(&bufDesc);
    uint32_t numElements = bufDesc.ByteWidth / 28;

    //Bind vertex buffers
    uint32_t stride = 28;
    uint32_t offset = 0;
    ctx->IASetVertexBuffers(0, 1, &pointBufferD3d, &stride, &offset);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    ctx->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
    ctx->Draw(numElements, 0);

    ctx->GSSetShader(nullptr, nullptr, 0);

    //Texture read back
    CD3D11_TEXTURE2D_DESC stgDesc{ DXGI_FORMAT_R32_UINT, RTWidth, RTHeight, 1, 1,
                                   0, D3D11_USAGE_STAGING, D3D11_CPU_ACCESS_READ };
    ComPtr<ID3D11Texture2D> StagingTex;
    dev->CreateTexture2D(&stgDesc, nullptr, StagingTex.GetAddressOf());

    ctx->CopyResource(StagingTex.Get(), RenderTexture.Get());
    D3D11_MAPPED_SUBRESOURCE mappedStgBuffer;
    ctx->Map(StagingTex.Get(), 0, D3D11_MAP_READ, 0, &mappedStgBuffer);
    const uint32_t* array = static_cast<const uint32_t*>(mappedStgBuffer.pData);
    uint32_t retId = array[y * mappedStgBuffer.RowPitch / 4 + x];
    ctx->Unmap(StagingTex.Get(), 0);
    return retId;
}

}
