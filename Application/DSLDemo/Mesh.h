#pragma once
#include "GraphicsDevice.h"
#include "DataTypeHelper.h"

#include <Vector3.h>
#include <wrl/client.h>
#include <vector>
#include <string>
#include <unordered_map>

namespace Nome
{

using Microsoft::WRL::ComPtr;

class CMeshAttribute
{
public:
	enum EBufferLocation
	{
		CPU = 1,
		GPU = 2
	};

	CMeshAttribute() : LocationFlags(0) {}

	CMeshAttribute(unsigned int location, std::vector<uint8_t> cpuBuf, ComPtr<ID3D11Buffer> gpuBuf, EDataType type)
		: LocationFlags(location), CPUBuffer(std::move(cpuBuf)), GPUBuffer(std::move(gpuBuf)), DataType(type)
	{
	}

	CMeshAttribute(std::vector<uint8_t> cpuBuf, EDataType type)
		: LocationFlags(CPU), CPUBuffer(std::move(cpuBuf)), GPUBuffer(), DataType(type)
	{
	}

	CMeshAttribute(ComPtr<ID3D11Buffer> gpuBuf, EDataType type)
		: LocationFlags(GPU), CPUBuffer(), GPUBuffer(std::move(gpuBuf)), DataType(type)
	{
	}

	EDataType GetDataType() const
	{
		return DataType;
	}

	bool IsOnGPU() const
	{
		return LocationFlags & GPU;
	}

	ID3D11Buffer* GetGPUBuffer() const
	{
		return GPUBuffer.Get();
	}

private:
	unsigned int LocationFlags;
	std::vector<uint8_t> CPUBuffer;
	ComPtr<ID3D11Buffer> GPUBuffer;
	EDataType DataType = EDataType::Invalid;
};

class CEffiUnindexedMesh
{
public:
	std::unordered_map<std::string, CMeshAttribute> Attributes;
	unsigned int NumVertices;
};

class CEffiMesh
{
public:
	CEffiMesh() : GD(nullptr) {}

	CEffiMesh(CGraphicsDevice* gd) : GD(gd) {}

	using index_t = uint32_t;

	struct AttributeBuffer
	{
		std::vector<uint8_t> AttrArr;
		ID3D11Buffer* GPUBuffer = nullptr;
		EDataType DataType = EDataType::Invalid;
	};

	///A map from names to attribute buffers
	std::unordered_map<std::string, AttributeBuffer> VertexAttrs;
	
	//never actually used, yet
	//std::unordered_map<std::string, AttributeBuffer> FaceAttrs;
	
	struct FaceData
	{
		std::vector<index_t> VertexIndexList;
	};
	
	std::vector<FaceData> Faces;
	ID3D11Buffer* TriangulatedIndexBuffer = nullptr;
	unsigned int NumIndices = 0;

	void SyncAttrToGPU(const std::string& name);
	void SyncIndicesToGPU();
	void SyncToGPU();

private:
	CGraphicsDevice* GD;
};

}
