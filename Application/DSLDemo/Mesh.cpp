#include "Mesh.h"

namespace Nome
{

template <typename T>
void SafeRelease(T& refP)
{
	if (refP)
	{
		refP->Release();
		refP = nullptr;
	}
}

template <typename TData>
void UploadVector(ID3D11Device* device, const std::vector<TData>& vec,
	D3D11_USAGE usage, UINT bindFlags, UINT cpuFlags, ID3D11Buffer** outBuffer)
{
	D3D11_BUFFER_DESC desc = {
		/* ByteWidth */ static_cast<UINT>(vec.size() * sizeof(TData)),
		/* Usage */ usage,
		/* BindFlags */ bindFlags,
		/* CPUAccessFlags */ cpuFlags,
		/* MiscFlags */ 0,
		/* StructureByteStride */ 0,
	};
	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vec.data();
	device->CreateBuffer(&desc, &initData, outBuffer);
}

void CEffiMesh::SyncAttrToGPU(const std::string& name)
{
	auto& vertAttr = VertexAttrs[name];

	if (vertAttr.GPUBuffer)
		return;
	SafeRelease(vertAttr.GPUBuffer);
	UploadVector(GD->GetDevice(), vertAttr.AttrArr, D3D11_USAGE_DEFAULT, D3D11_BIND_VERTEX_BUFFER, 0, &vertAttr.GPUBuffer);
}

void CEffiMesh::SyncIndicesToGPU()
{
	if (TriangulatedIndexBuffer)
		return;
	
	NumIndices = 0;

	SafeRelease(TriangulatedIndexBuffer);
	std::vector<index_t> Triangulate;
	for (const auto& faceData : Faces)
	{
		for (size_t i = 1; i < faceData.VertexIndexList.size() - 1; i++)
		{
			Triangulate.push_back(faceData.VertexIndexList[0]);
			Triangulate.push_back(faceData.VertexIndexList[i]);
			Triangulate.push_back(faceData.VertexIndexList[i + 1]);
			NumIndices += 3;
		}
	}
	UploadVector(GD->GetDevice(), Triangulate, D3D11_USAGE_DEFAULT, D3D11_BIND_INDEX_BUFFER, 0, &TriangulatedIndexBuffer);
}

void CEffiMesh::SyncToGPU()
{
	for (auto& pair : VertexAttrs)
	{
		SyncAttrToGPU(pair.first);
	}
	SyncIndicesToGPU();
}

} // namespace Nome
