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
	: GD(gd)
{
	CHLSLCodeGen codegen;
	auto shaderFunc = codegen.CodeGen(targetExpr, "MaterializeAttr");

	CHLSLInputStructGen vsinGen{ codegen.ReferredFields };
	auto shaderVSIn = vsinGen.Result;

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
	ID3D11InputLayout* inputLayout;
	GD->GetDevice()->CreateInputLayout(inputElements.data(), (UINT)inputElements.size(),
		CodeBlob->GetBufferPointer(), CodeBlob->GetBufferSize(), &inputLayout);
	if (!inputLayout)
		throw CEffiCompileError(tc::StringPrintf("Attribute %s cannot be materialized due to input layout error.",
			attrName.c_str()));

	ID3D11GeometryShader* geometryShader;
	D3D11_SO_DECLARATION_ENTRY soDecl;
	soDecl.Stream = 0;
	soDecl.SemanticName = "TEXCOORD"; //See above
	soDecl.SemanticIndex = 0;
	soDecl.StartComponent = 0;
	soDecl.ComponentCount = (BYTE)DataTypeToSize(targetExpr->DataType) / 4; //4 bytes per component
	soDecl.OutputSlot = 0;
	UINT soBufferStride = (UINT)DataTypeToSize(targetExpr->DataType);
	GD->GetDevice()->CreateGeometryShaderWithStreamOutput(CodeBlob->GetBufferPointer(), CodeBlob->GetBufferSize(),
		&soDecl, 1, &soBufferStride, 1, D3D11_SO_NO_RASTERIZED_STREAM, nullptr, &geometryShader);
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
		const std::string& referred = pair.first;
		EDataType dataType = pair.second;

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

		attrBuffers.push_back(iter->second.GPUBuffer);
		offsets.push_back(0);
		strides.push_back((UINT)DataTypeToSize(iter->second.DataType));
	}

	//TODO:
	//device->CreateInputLayout(inputElements.data(), inputElements.size(), nullptr, 0, nullptr);
}

} /* namespace Nome */
