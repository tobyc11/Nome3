#include "MOMaterializeAttr.h"
#include "ConvertToHLSLType.h"
#include <StringPrintf.h>

namespace Nome
{

MOMaterializeAttr::MOMaterializeAttr(CGraphicsDevice* gd, const std::string& mapFunc, const std::string& vsIn, const std::string& vsOut)
{
}

void MOMaterializeAttr::operator()(CEffiMesh& mesh)
{
	auto* device = GD->GetDevice();
	auto* ctx = GD->GetImmediateContext();

	std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements;
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

		D3D11_INPUT_ELEMENT_DESC desc = {
			"ATTRIBUTE", //LPCSTR SemanticName;
			(UINT)inputElements.size(), //UINT SemanticIndex;
			ConvertToDXGIFormat(iter->second.DataType), //DXGI_FORMAT Format;
			(UINT)attrBuffers.size(), //UINT InputSlot;
			0, //UINT AlignedByteOffset;
			D3D11_INPUT_PER_VERTEX_DATA, //D3D11_INPUT_CLASSIFICATION InputSlotClass;
			0, //UINT InstanceDataStepRate;
		};
		inputElements.push_back(desc);
		attrBuffers.push_back(iter->second.GPUBuffer);
		offsets.push_back(0);
		strides.push_back((UINT)DataTypeToSize(iter->second.DataType));
	}

	//TODO:
	//device->CreateInputLayout(inputElements.data(), inputElements.size(), nullptr, 0, nullptr);
}

} /* namespace Nome */
