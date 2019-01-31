#include "Renderer.h"
#include "GraphicsDevice.h"
#include "Geometry.h"
#include "Material.h"
#include "Viewport.h"
#include "ShaderCommon.h"

namespace Nome
{

static CRenderer StaticRenderer;
CRenderer* GRenderer = &StaticRenderer;

CRenderer::CRenderer()
{
	GD = new CGraphicsDevice();
}

CRenderer::~CRenderer()
{
	delete GD;
}

void CRenderer::BeginView(const tc::Matrix4& view, const tc::Matrix4& proj, CViewport* viewport, const tc::Color& clearColor)
{
    CViewData viewData;
    viewData.ViewMat = view;
    viewData.ProjMat = proj;
	viewData.Viewport = viewport;
	viewData.ClearColor = clearColor;
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
        std::ifstream ifs("Resources/Basic.hlsl");
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
		CompileFile("Resources/Wire.hlsl", "VSmain", "vs_5_0");
		//Create input layout
		D3D11_INPUT_ELEMENT_DESC ilDesc[] = {
			{ "ATTRIBUTE", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		dev->CreateInputLayout(ilDesc, sizeof(ilDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC),
			VSBytecode->GetBufferPointer(), VSBytecode->GetBufferSize(), DefaultInputLayout.GetAddressOf());
		dev->CreateVertexShader(VSBytecode->GetBufferPointer(), VSBytecode->GetBufferSize(), nullptr, VS.GetAddressOf());
		CompileFile("Resources/Wire.hlsl", "GSmain", "gs_5_0");
		dev->CreateGeometryShader(GSBytecode->GetBufferPointer(), GSBytecode->GetBufferSize(), nullptr, GS.GetAddressOf());
		GSBytecode = nullptr;
		CompileFile("Resources/Wire.hlsl", "PSmain", "ps_5_0");
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

void CRenderer::Render()
{
    //Once upon a time, there was drawing policy and shader manager and such...
    //  But I am lazy.
    if (!BasicShader)
        BasicShader.reset(new CBasicShader());
	if (!WireShader)
		WireShader.reset(new CWireShader());

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
				cbEverything.LineWidth = 2.5;
				cbEverything.Width = view.Viewport->GetWidth();
				cbEverything.Height = view.Viewport->GetHeight();
				WireShader->UpdateCBEverything(ctx);
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
    }
	ctx->GSSetShader(nullptr, nullptr, 0);
	Views.clear();
}

}
