#include "MOSubdivideAdHoc.h"
#include "ConvertToHLSLType.h"
#include "EffiCompiler.h"
#include "ShaderManager.h"
#include <StringPrintf.h>

#include <d3dcompiler.h>
#include <utility>
#include <sstream>
#include <iostream>
#include <fstream>

namespace Nome
{

MOSubdivideAdHoc::MOSubdivideAdHoc(CGraphicsDevice* gd, IRExpr* inputExpr) : GD(gd)
{
	if (inputExpr->DataType != EDataType::Float3)
		throw CEffiCompileError("AdHoc subdivision must be fed float3");

	CHLSLCodeGen codegen;
	auto shaderFunc = codegen.CodeGen(inputExpr, "CalcPos");

	CHLSLInputStructGen vsinGen{ codegen.ReferredFields };
	auto shaderVSIn = vsinGen.Result;

	//Rearrange the input attrs
	for (const auto& field : codegen.ReferredFields)
		ReferredAttrs.insert({ field.second.second, { field.first, field.second.first } });

	std::unordered_map<std::string, std::pair<EDataType, std::string>> outVars;
	outVars.insert({ "Pos", {inputExpr->DataType, "POSITION"} });
	CHLSLStructGen vsOutGen{ "VSOut", outVars };
	auto shaderVSOut = vsOutGen.Result;

	//Generate the full shader
	std::stringstream ss;
	ss << shaderVSIn << std::endl;
	ss << shaderVSOut << std::endl;
	ss << shaderFunc << std::endl;

	//Read the shader template from disk
	std::ifstream ifs("Resources/adhoc_subdiv.hlsl");
	std::string str;

	ifs.seekg(0, std::ios::end);
	str.reserve(ifs.tellg());
	ifs.seekg(0, std::ios::beg);

	str.assign((std::istreambuf_iterator<char>(ifs)),
		std::istreambuf_iterator<char>());

	ss << str << std::endl;
	std::cout << "Generated shader:" << std::endl;
	std::cout << ss.str() << std::endl;

	//Compile shader
	auto sourceStr = ss.str();
	ID3DBlob* vsCode;
	ID3DBlob* gsCode;
	ID3DBlob* ErrorBlob;
	D3DCompile(sourceStr.c_str(), sourceStr.size(), "adhoc", nullptr, nullptr, "VSmain", "vs_5_0", 0, 0, &vsCode, &ErrorBlob);
	if (ErrorBlob)
	{
		std::cout << "[VS] Shader compilation error:" << std::endl;
		std::cout << reinterpret_cast<const char*>(ErrorBlob->GetBufferPointer()) << std::endl;
		ErrorBlob->Release();

		throw CEffiCompileError("Shader compilation error.");
	}
	D3DCompile(sourceStr.c_str(), sourceStr.size(), "adhoc", nullptr, nullptr, "GSmain", "gs_5_0", 0, 0, &gsCode, &ErrorBlob);
	if (ErrorBlob)
	{
		std::cout << "[GS] Shader compilation error:" << std::endl;
		std::cout << reinterpret_cast<const char*>(ErrorBlob->GetBufferPointer()) << std::endl;
		ErrorBlob->Release();

		throw CEffiCompileError("Shader compilation error.");
	}

	//Create input signature
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements;
	for (const auto& field : codegen.ReferredFields)
	{
		D3D11_INPUT_ELEMENT_DESC desc = {
			"ATTRIBUTE", //LPCSTR SemanticName;
			(UINT)field.second.second, //UINT SemanticIndex;
			ConvertToDXGIFormat(field.second.first), //DXGI_FORMAT Format;
			(UINT)field.second.second, //UINT InputSlot;
			0, //UINT AlignedByteOffset;
			D3D11_INPUT_PER_VERTEX_DATA, //D3D11_INPUT_CLASSIFICATION InputSlotClass;
			0, //UINT InstanceDataStepRate;
		};
		inputElements.push_back(desc);
	}
	GD->GetDevice()->CreateInputLayout(inputElements.data(), (UINT)inputElements.size(),
		vsCode->GetBufferPointer(), vsCode->GetBufferSize(), &InputLayout);
	if (!InputLayout)
		throw CEffiCompileError("Input layout cannot be created.");

	GD->GetDevice()->CreateVertexShader(vsCode->GetBufferPointer(), vsCode->GetBufferSize(), nullptr, &VertexShader);

	D3D11_SO_DECLARATION_ENTRY soDecl;
	soDecl.Stream = 0;
	soDecl.SemanticName = "POSITION"; //See above
	soDecl.SemanticIndex = 0;
	soDecl.StartComponent = 0;
	soDecl.ComponentCount = (BYTE)DataTypeToSize(inputExpr->DataType) / 4; //4 bytes per component
	soDecl.OutputSlot = 0;
	UINT soBufferStride = (UINT)DataTypeToSize(inputExpr->DataType);
	GD->GetDevice()->CreateGeometryShaderWithStreamOutput(gsCode->GetBufferPointer(), gsCode->GetBufferSize(),
		&soDecl, 1, &soBufferStride, 1, D3D11_SO_NO_RASTERIZED_STREAM, nullptr, &GeometryShader);
}

CEffiUnindexedMesh* MOSubdivideAdHoc::operator()(CEffiMesh& mesh)
{
	auto* device = GD->GetDevice();
	auto* ctx = GD->GetImmediateContext();

	std::vector<ID3D11Buffer*> attrBuffers;
	std::vector<UINT> offsets;
	std::vector<UINT> strides;

	//Make sure each of the referred attributes actually exist, and types match
	for (const auto& pair : ReferredAttrs)
	{
		const std::string& referred = pair.second.first;
		EDataType dataType = pair.second.second;

		auto iter = mesh.VertexAttrs.find(referred);
		if (iter == mesh.VertexAttrs.end())
		{
			//Doesn't exist in mesh
			throw CMeshOpException(tc::StringPrintf("Attribute %s not found in mesh.", referred.c_str()));
		}

		//Found
		if (iter->second.DataType != dataType)
		{
			throw CMeshOpException(tc::StringPrintf("Attribute %s type mismatch.", referred.c_str()));
		}

		mesh.SyncAttrToGPU(referred);

		attrBuffers.push_back(iter->second.GPUBuffer);
		offsets.push_back(0);
		strides.push_back((UINT)DataTypeToSize(iter->second.DataType));
	}

	mesh.SyncIndicesToGPU();

	ctx->IASetInputLayout(InputLayout);
	ctx->IASetVertexBuffers(0, 1, attrBuffers.data(), strides.data(), offsets.data());
	ctx->IASetIndexBuffer(mesh.TriangulatedIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ctx->VSSetShader(VertexShader, nullptr, 0);
	ctx->GSSetShader(GeometryShader, nullptr, 0);

	D3D11_BUFFER_DESC desc =
	{
		(UINT)DataTypeToSize(EDataType::Float3) * mesh.NumIndices * 10,
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_STREAM_OUTPUT | D3D11_BIND_VERTEX_BUFFER,
		0,
		0,
		0
	};
	ID3D11Buffer* outputBuffer;
	device->CreateBuffer(&desc, nullptr, &outputBuffer);

	UINT soOffset = 0;
	ctx->SOSetTargets(1, &outputBuffer, &soOffset);

	ctx->DrawIndexed(mesh.NumIndices, 0, 0);
	ctx->GSSetShader(nullptr, nullptr, 0);
	ctx->SOSetTargets(0, nullptr, nullptr);

	CEffiUnindexedMesh* output = new CEffiUnindexedMesh();
	output->NumVertices = mesh.NumIndices * 3;
	output->Attributes.emplace("pos", CMeshAttribute(outputBuffer, EDataType::Float3));
	return output;
}

CEffiUnindexedMesh* MOSubdivideAdHoc::operator()(CEffiUnindexedMesh& mesh)
{
	auto* device = GD->GetDevice();
	auto* ctx = GD->GetImmediateContext();

	std::vector<ID3D11Buffer*> attrBuffers;
	std::vector<UINT> offsets;
	std::vector<UINT> strides;

	//Make sure each of the referred attributes actually exist, and types match
	for (const auto& pair : ReferredAttrs)
	{
		const std::string& referred = pair.second.first;
		EDataType dataType = pair.second.second;

		auto iter = mesh.Attributes.find(referred);
		if (iter == mesh.Attributes.end())
		{
			//Doesn't exist in mesh
			throw CMeshOpException(tc::StringPrintf("Attribute %s not found in mesh.", referred.c_str()));
		}

		//Found
		if (iter->second.GetDataType() != dataType)
		{
			throw CMeshOpException(tc::StringPrintf("Attribute %s type mismatch.", referred.c_str()));
		}
		if (!iter->second.IsOnGPU())
		{
			throw CMeshOpException(tc::StringPrintf("Attribute %s is not on GPU.", referred.c_str()));
		}

		attrBuffers.push_back(iter->second.GetGPUBuffer());
		offsets.push_back(0);
		strides.push_back((UINT)DataTypeToSize(iter->second.GetDataType()));
	}

	ctx->IASetInputLayout(InputLayout);
	ctx->IASetVertexBuffers(0, 1, attrBuffers.data(), strides.data(), offsets.data());
	ctx->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ctx->VSSetShader(VertexShader, nullptr, 0);
	ctx->GSSetShader(GeometryShader, nullptr, 0);

	D3D11_BUFFER_DESC desc =
	{
		(UINT)DataTypeToSize(EDataType::Float3) * mesh.NumVertices * 10,
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_STREAM_OUTPUT,
		0,
		0,
		0
	};
	ID3D11Buffer* outputBuffer;
	device->CreateBuffer(&desc, nullptr, &outputBuffer);

	UINT soOffset = 0;
	ctx->SOSetTargets(1, &outputBuffer, &soOffset);

	ctx->Draw(mesh.NumVertices, 0);
	ctx->GSSetShader(nullptr, nullptr, 0);
	ctx->SOSetTargets(0, nullptr, nullptr);

	CEffiUnindexedMesh* output = new CEffiUnindexedMesh();
	output->NumVertices = mesh.NumVertices * 3;
	output->Attributes = mesh.Attributes;
	output->Attributes.insert_or_assign("pos", CMeshAttribute(outputBuffer, EDataType::Float3));
	return output;
}

} /* namespace Nome */
