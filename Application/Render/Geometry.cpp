#include "Geometry.h"
#include "Renderer.h"
#include "GraphicsDevice.h"

namespace Nome
{

CVertexBuffer::CVertexBuffer(size_t size) : CPUSize(size)
{
	CPUBuffer = malloc(size);
}

CVertexBuffer::CVertexBuffer(size_t size, void* data) : CPUSize(0)
{
	ID3D11Device* dev = GRenderer->GetGD()->GetDevice();
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = (UINT)size;
	desc.StructureByteStride = 0;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = data;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;
	dev->CreateBuffer(&desc, &initData, GPUBuffer.GetAddressOf());
}

CVertexBuffer::~CVertexBuffer()
{
	if (CPUBuffer)
		free(CPUBuffer);
}

bool CVertexBuffer::IsOnGPU() const
{
	return GPUBuffer.operator Microsoft::WRL::Details::BoolType();
}

void CVertexBuffer::UploadToGPU()
{
	if (IsOnGPU())
		return;

	//TODO
}

CIndexBuffer::CIndexBuffer(size_t size, size_t elementWidth, void * data) : ElementWidth(elementWidth)
{
	ID3D11Device* dev = GRenderer->GetGD()->GetDevice();
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = (UINT)size;
	desc.StructureByteStride = 0;
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = data;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;
	dev->CreateBuffer(&desc, &initData, GPUBuffer.GetAddressOf());
}

CStaticMeshGeometry::CAttribute::CAttribute(const std::string& name, uint32_t index, DXGI_FORMAT format,
	CVertexBuffer* buffer, uint32_t offset, uint32_t stride)
	: Name(name), Index(index), Format(format), Buffer(buffer), Offset(offset), Stride(stride)
{
}

CStaticMeshGeometry::CStaticMeshGeometry()
{
}

CStaticMeshGeometry::CStaticMeshGeometry(const CNomeMesh& fromMesh)
{
	Reconstruct(fromMesh);
}

void CStaticMeshGeometry::Reconstruct(const CNomeMesh & fromMesh)
{
	IB = nullptr;
	Attribs.clear();

	struct CVertexData
	{
		Vector3 Pos;
	};

	static_assert(sizeof(CVertexData) == 12, "Vertex size is unexpected");

	std::vector<CVertexData> vertBuffer;
	vertBuffer.resize(fromMesh.n_vertices());
	CNomeMesh::VertexIter vIter, vEnd = fromMesh.vertices_end();
	for (vIter = fromMesh.vertices_begin(); vIter != vEnd; ++vIter)
	{
		CVertexData& data = vertBuffer[(*vIter).idx()];
		data.Pos.x = fromMesh.point(*vIter)[0];
		data.Pos.y = fromMesh.point(*vIter)[1];
		data.Pos.z = fromMesh.point(*vIter)[2];
	}

	TAutoPtr<CVertexBuffer> vb = new CVertexBuffer(vertBuffer.size() * sizeof(CVertexData), vertBuffer.data());
	Attribs.emplace_back("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, vb, 0, (uint32_t)sizeof(CVertexData));

	std::vector<uint32_t> indexBuffer;
	CNomeMesh::FaceIter fIter, fEnd = fromMesh.faces_end();
	for (fIter = fromMesh.faces_begin(); fIter != fEnd; ++fIter)
	{
		int v0, vPrev, vCurr;
		int faceVCount = 0;
		CNomeMesh::FaceVertexIter fvIter = CNomeMesh::FaceVertexIter(fromMesh, *fIter);
		for (; fvIter.is_valid(); ++fvIter)
		{
			CNomeMesh::VertexHandle faceVert = *fvIter;
			if (faceVCount == 0)
			{
				v0 = faceVert.idx();
			}
			else if (faceVCount == 1)
			{
				vPrev = faceVert.idx();
			}
			else
			{
				vCurr = faceVert.idx();
				indexBuffer.push_back(v0);
				indexBuffer.push_back(vPrev);
				indexBuffer.push_back(vCurr);
				vPrev = vCurr;
			}
			faceVCount++;
		}
	}
	IB = new CIndexBuffer(indexBuffer.size() * sizeof(uint32_t), sizeof(uint32_t), indexBuffer.data());
}

}
