#include "DSLDemoRenderer.h"

#include "Nome/App.h"
#include "Nome/SDLService.h"
#include "ShaderManager.h"

#include <fstream>

namespace Nome
{

static ID3D11InputLayout* InputLayout;
static ID3D11VertexShader* VertexShader;
static ID3D11PixelShader* PixelShader;
static ID3D11RasterizerState* RSWireFrame;
static ID3D11DepthStencilState* DepthStencilState;
static ID3D11BlendState* BlendState;

DEFINE_CBUFFER(CBEverything, 0)
DEFINE_CBUFFER_MEMBER(Matrix4, Model)
DEFINE_CBUFFER_MEMBER(Matrix4, View)
DEFINE_CBUFFER_MEMBER(Matrix4, Proj)
END_CBUFFER()

static CBEverything EverythingBuffer;
static ID3D11Buffer* EverythingDXBuffer;

void CMeshRenderer::Render(const Scene::CCamera& camera)
{
	if (Mesh)
	{
		auto* rc = GApp->GetService<CSDLService>()->RenderContext;
		auto* gd = rc->GetGraphicsDevice();
		auto* ctx = gd->GetImmediateContext();

		Mesh->SyncToGPU();

		ctx->IASetInputLayout(InputLayout);
		auto* buffer = Mesh->VertexAttrs["pos"].GPUBuffer;
		UINT stride = DataTypeToSize(Mesh->VertexAttrs["pos"].DataType);
		UINT offset = 0;
		ctx->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
		ctx->IASetIndexBuffer(Mesh->TriangulatedIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		ctx->VSSetShader(VertexShader, nullptr, 0);
		ID3D11Buffer* bufsToBind[] = { EverythingDXBuffer };
		ctx->VSSetConstantBuffers(0, 1, bufsToBind);

		D3D11_MAPPED_SUBRESOURCE map;
		ctx->Map(EverythingDXBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
		CBEverything* cbEverything = (CBEverything*)map.pData;
		cbEverything->Model = Matrix4::IDENTITY;
		cbEverything->View = camera.GetViewMatrix().Transpose();
		cbEverything->Proj = camera.GetProjMatrix().Transpose();
		ctx->Unmap(EverythingDXBuffer, 0);

		ctx->RSSetState(RSWireFrame);

		ctx->PSSetShader(PixelShader, nullptr, 0);

		ctx->OMSetDepthStencilState(DepthStencilState, 0);
		ctx->OMSetBlendState(BlendState, nullptr, 0xffffffff);

		ctx->DrawIndexed(Mesh->NumIndices, 0, 0);
	}
	else if (UnidxMesh)
	{
		auto* rc = GApp->GetService<CSDLService>()->RenderContext;
		auto* gd = rc->GetGraphicsDevice();
		auto* ctx = gd->GetImmediateContext();

		ctx->IASetInputLayout(InputLayout);
		auto* buffer = UnidxMesh->Attributes["pos"].GetGPUBuffer();
		UINT stride = DataTypeToSize(UnidxMesh->Attributes["pos"].GetDataType());
		UINT offset = 0;
		ctx->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
		ctx->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);
		ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		ctx->VSSetShader(VertexShader, nullptr, 0);
		ID3D11Buffer* bufsToBind[] = { EverythingDXBuffer };
		ctx->VSSetConstantBuffers(0, 1, bufsToBind);

		ctx->SOSetTargets(0, nullptr, nullptr);

		D3D11_MAPPED_SUBRESOURCE map;
		ctx->Map(EverythingDXBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
		CBEverything* cbEverything = (CBEverything*)map.pData;
		cbEverything->Model = Matrix4::IDENTITY;
		cbEverything->View = camera.GetViewMatrix().Transpose();
		cbEverything->Proj = camera.GetProjMatrix().Transpose();
		ctx->Unmap(EverythingDXBuffer, 0);

		ctx->RSSetState(RSWireFrame);

		ctx->PSSetShader(PixelShader, nullptr, 0);

		ctx->OMSetDepthStencilState(DepthStencilState, 0);
		ctx->OMSetBlendState(BlendState, nullptr, 0xffffffff);

		ctx->Draw(UnidxMesh->NumVertices, 0);
	}
}

int CDSLDemoRenderer::Setup()
{
	auto* rc = GApp->GetService<CSDLService>()->RenderContext;
	auto* gd = rc->GetGraphicsDevice();
	auto* sm = gd->GetShaderManager();
	ID3D11Device* dev = gd->GetDevice();

	//EverythingDXBuffer = EverythingBuffer._Meta.CreateConstantBuffer(dev);
	//auto generatedCBDecl = EverythingBuffer._Meta.ToString();
	//std::ofstream ofs("Resources/Generated.h");
	//ofs << generatedCBDecl << std::endl;
	//ofs.close();
	D3D11_BUFFER_DESC desc = {
		/* ByteWidth */ static_cast<UINT>(sizeof(CBEverything)),
		/* Usage */ D3D11_USAGE_DYNAMIC,
		/* BindFlags */ D3D11_BIND_CONSTANT_BUFFER,
		/* CPUAccessFlags */ D3D11_CPU_ACCESS_WRITE,
		/* MiscFlags */ 0,
		/* StructureByteStride */ 0,
	};
	ID3D11Buffer* result;
	dev->CreateBuffer(&desc, nullptr, &EverythingDXBuffer);

	ID3DBlob* vsCode = nullptr;
	sm->CompileFromFile("Resources/pos_only_white.hlsl", nullptr, "VSmain", "vs_5_0", 0, 0, &vsCode);
	ID3DBlob* psCode = nullptr;
	sm->CompileFromFile("Resources/pos_only_white.hlsl", nullptr, "PSmain", "ps_5_0", 0, 0, &psCode);

	D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] = {
		{ "ATTRIBUTE", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	dev->CreateInputLayout(inputLayoutDesc, 1, vsCode->GetBufferPointer(), vsCode->GetBufferSize(), &InputLayout);
	
	dev->CreateVertexShader(vsCode->GetBufferPointer(), vsCode->GetBufferSize(), nullptr, &VertexShader);

	dev->CreatePixelShader(psCode->GetBufferPointer(), psCode->GetBufferSize(), nullptr, &PixelShader);

	// Create the blending setup
	{
		D3D11_BLEND_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.AlphaToCoverageEnable = false;
		desc.RenderTarget[0].BlendEnable = false;
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		dev->CreateBlendState(&desc, &BlendState);
	}

	// Create the rasterizer state
	{
		D3D11_RASTERIZER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.FillMode = D3D11_FILL_WIREFRAME;
		desc.CullMode = D3D11_CULL_NONE;
		desc.ScissorEnable = false;
		desc.DepthClipEnable = false;
		dev->CreateRasterizerState(&desc, &RSWireFrame);
	}

	// Create depth-stencil State
	{
		D3D11_DEPTH_STENCIL_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.DepthEnable = false;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
		desc.StencilEnable = false;
		desc.FrontFace.StencilFailOp = desc.FrontFace.StencilDepthFailOp = desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		desc.BackFace = desc.FrontFace;
		dev->CreateDepthStencilState(&desc, &DepthStencilState);
	}

	return 0;
}

int CDSLDemoRenderer::FrameUpdate()
{
	return 0;
}

int CDSLDemoRenderer::Cleanup()
{
	InputLayout->Release();
	VertexShader->Release();
	PixelShader->Release();
	RSWireFrame->Release();
	return 0;
}

void CDSLDemoRenderer::Render()
{
	MeshRenderer.Render(*Camera);
}

} /* namespace Nome */
