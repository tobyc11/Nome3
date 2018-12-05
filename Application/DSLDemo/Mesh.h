#pragma once
#include "GraphicsDevice.h"
#include "DataTypeHelper.h"

#include <Vector3.h>

#include <vector>
#include <string>
#include <unordered_map>

namespace Nome
{

class CEffiMesh
{
public:
	using index_t = uint32_t;

	struct AttributeBuffer
	{
		std::vector<uint8_t> AttrArr;
		ID3D11Buffer* GPUBuffer;
		EDataType DataType;
	};

	std::unordered_map<std::string, AttributeBuffer> VertexAttrs;
	std::unordered_map<std::string, AttributeBuffer> FaceAttrs;
	
	struct FaceData
	{
		std::vector<index_t> VertexIndexList;
	};
	
	std::vector<FaceData> Faces;
	ID3D11Buffer* TriangulatedIndexBuffer;

	void SyncAttrToGPU(const std::string& name);
	void SyncToGPU();

private:
	CGraphicsDevice* GD;
};

}
