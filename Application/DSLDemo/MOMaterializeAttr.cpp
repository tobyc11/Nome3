#include "MOMaterializeAttr.h"
#include "ConvertToHLSLType.h"
#include "EffiCompiler.h"
#include <StringPrintf.h>

#include <d3dcompiler.h>
#include <utility>
#include <sstream>
#include <iostream>

namespace Nome
{

MOMaterializeAttr::MOMaterializeAttr(CGraphicsDevice* gd, const std::string& attrName, IRExpr* targetExpr)
	: GD(gd), AttrName(attrName)
{
	CHLSLCodeGen codegen;
	auto shaderFunc = codegen.CodeGen(targetExpr, "MaterializeAttr");

	CHLSLInputStructGen vsinGen{ codegen.ReferredFields };
	auto shaderVSIn = vsinGen.Result;

	//Rearrange the input attrs
	for (const auto& field : codegen.ReferredFields)
		ReferredAttrs.insert({ field.second.second, { field.first, field.second.first } });
	OutputType = targetExpr->DataType;

	std::unordered_map<std::string, std::pair<EDataType, std::string>> outVars;
	outVars.insert({ "Pos", {targetExpr->DataType, "TEXCOORD0"} });
	CHLSLStructGen vsOutGen{ "VSOut", outVars };
	auto shaderVSOut = vsOutGen.Result;

	//Generate the full shader
	std::stringstream ss;
	ss << shaderVSIn << std::endl;
	ss << shaderVSOut << std::endl;
	ss << shaderFunc << std::endl;
	ss << tc::StringPrintf("VSOut VSmain(VSIn input)\n"
		"{\n"
		"	VSOut output;\n"
		"	output.Pos = MaterializeAttr(input);\n"
		"	return output;\n"
		"}\n");

	std::cout << "Generated shader:" << std::endl;
	std::cout << ss.str() << std::endl;

	//Compile shader
	auto sourceStr = ss.str();
	ID3DBlob* CodeBlob;
	ID3DBlob* ErrorBlob;
	D3DCompile(sourceStr.c_str(), sourceStr.size(), "materialize_vs", nullptr, nullptr, "VSmain", "vs_5_0", 0, 0, &CodeBlob, &ErrorBlob);
	if (ErrorBlob)
	{
		std::cout << "Shader compilation error:" << std::endl;
		std::cout << reinterpret_cast<const char*>(ErrorBlob->GetBufferPointer()) << std::endl;
		ErrorBlob->Release();

		throw CEffiCompileError(tc::StringPrintf("Attribute %s cannot be materialized due to shader compilation error.",
			attrName.c_str()));
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
		CodeBlob->GetBufferPointer(), CodeBlob->GetBufferSize(), &InputLayout);
	if (!InputLayout)
		throw CEffiCompileError(tc::StringPrintf("Attribute %s cannot be materialized due to input layout error.",
			attrName.c_str()));

	GD->GetDevice()->CreateVertexShader(CodeBlob->GetBufferPointer(), CodeBlob->GetBufferSize(), nullptr, &VertexShader);

	D3D11_SO_DECLARATION_ENTRY soDecl;
	soDecl.Stream = 0;
	soDecl.SemanticName = "TEXCOORD"; //See above
	soDecl.SemanticIndex = 0;
	soDecl.StartComponent = 0;
	soDecl.ComponentCount = (BYTE)DataTypeToSize(targetExpr->DataType) / 4; //4 bytes per component
	soDecl.OutputSlot = 0;
	UINT soBufferStride = (UINT)DataTypeToSize(targetExpr->DataType);
	GD->GetDevice()->CreateGeometryShaderWithStreamOutput(CodeBlob->GetBufferPointer(), CodeBlob->GetBufferSize(),
		&soDecl, 1, &soBufferStride, 1, D3D11_SO_NO_RASTERIZED_STREAM, nullptr, &GeometryShader);
}

void MOMaterializeAttr::operator()(CEffiMesh& mesh)
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
		DataTypeToSize(OutputType) * mesh.NumIndices * 10,
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

	mesh.VertexAttrs[AttrName].GPUBuffer->Release();
	mesh.VertexAttrs[AttrName].GPUBuffer = outputBuffer;
}

} /* namespace Nome */
