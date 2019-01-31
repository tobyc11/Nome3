#include "Geometry.h"
#include "Renderer.h"
#include "GraphicsDevice.h"

namespace Nome
{

CVertexBuffer::CVertexBuffer(size_t size) : CPUSize(size)
{
	CPUBuffer = malloc(size);
}

CVertexBuffer::CVertexBuffer(size_t size, const void* data) : CPUSize(0)
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

CStaticMeshGeometry::CStaticMeshGeometry(const CMeshImpl& fromMesh)
{
	Reconstruct(fromMesh);
}

CStaticMeshGeometry::CStaticMeshGeometry(const std::vector<Vector3>& positions)
{
	TAutoPtr<CVertexBuffer> vb = new CVertexBuffer(positions.size() * sizeof(Vector3), positions.data());
	Attribs.emplace_back("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, vb, 0, (uint32_t)sizeof(Vector3));
	ElementCount = (uint32_t)positions.size();
}

void CStaticMeshGeometry::Reconstruct(const CMeshImpl& fromMesh)
{
	IB = nullptr;
	Attribs.clear();

	struct CVertexData
	{
		Vector3 Pos;
		Vector3 Normal;
	};

	static_assert(sizeof(CVertexData) == 24, "Vertex size is unexpected");

	if (false)
	{
		//Generate vertex buffer data from every vertex
		std::vector<CVertexData> vertBuffer;
		vertBuffer.resize(fromMesh.n_vertices());
		CMeshImpl::VertexIter vIter, vEnd = fromMesh.vertices_end();
		for (vIter = fromMesh.vertices_begin(); vIter != vEnd; ++vIter)
		{
			CVertexData& data = vertBuffer[(*vIter).idx()];
			data.Pos.x = fromMesh.point(*vIter)[0];
			data.Pos.y = fromMesh.point(*vIter)[1];
			data.Pos.z = fromMesh.point(*vIter)[2];
			//Per vertex normal
			const auto& nv = fromMesh.vertex_normals()[vIter->idx()];
			data.Normal = { nv[0], nv[1], nv[2] };
		}

		TAutoPtr<CVertexBuffer> vb = new CVertexBuffer(vertBuffer.size() * sizeof(CVertexData), vertBuffer.data());
		Attribs.emplace_back("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, vb, 0, (uint32_t)sizeof(CVertexData));
		Attribs.emplace_back("NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, vb, 12, (uint32_t)sizeof(CVertexData));

		//Generate index buffer data from every face
		std::vector<uint32_t> indexBuffer;
		CMeshImpl::FaceIter fIter, fEnd = fromMesh.faces_end();
		for (fIter = fromMesh.faces_sbegin(); fIter != fEnd; ++fIter)
		{
			int v0, vPrev, vCurr;
			int faceVCount = 0;
			CMeshImpl::FaceVertexIter fvIter = CMeshImpl::FaceVertexIter(fromMesh, *fIter);
			for (; fvIter.is_valid(); ++fvIter)
			{
				CMeshImpl::VertexHandle faceVert = *fvIter;
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
		ElementCount = (uint32_t)indexBuffer.size();
		IB = new CIndexBuffer(indexBuffer.size() * sizeof(uint32_t), sizeof(uint32_t), indexBuffer.data());
	}
	else
	{
		//Per face normal, thus no shared vertices between faces
		std::vector<CVertexData> vertBuffer;
		CMeshImpl::FaceIter fIter, fEnd = fromMesh.faces_end();
		for (fIter = fromMesh.faces_sbegin(); fIter != fEnd; ++fIter)
		{
			CVertexData v0, vPrev, vCurr;
			int faceVCount = 0;
			CMeshImpl::FaceVertexIter fvIter = CMeshImpl::FaceVertexIter(fromMesh, *fIter);
			for (; fvIter.is_valid(); ++fvIter)
			{
				CMeshImpl::VertexHandle faceVert = *fvIter;
				if (faceVCount == 0)
				{
					const auto& posVec = fromMesh.point(faceVert);
					v0.Pos = { posVec[0], posVec[1], posVec[2] };
					const auto& fnVec = fromMesh.normal(*fIter);
					v0.Normal = { fnVec[0], fnVec[1], fnVec[2] };
				}
				else if (faceVCount == 1)
				{
					const auto& posVec = fromMesh.point(faceVert);
					vPrev.Pos = { posVec[0], posVec[1], posVec[2] };
					const auto& fnVec = fromMesh.normal(*fIter);
					vPrev.Normal = { fnVec[0], fnVec[1], fnVec[2] };
				}
				else
				{
					const auto& posVec = fromMesh.point(faceVert);
					vCurr.Pos = { posVec[0], posVec[1], posVec[2] };
					const auto& fnVec = fromMesh.normal(*fIter);
					vCurr.Normal = { fnVec[0], fnVec[1], fnVec[2] };
					vertBuffer.push_back(v0);
					vertBuffer.push_back(vPrev);
					vertBuffer.push_back(vCurr);
					vPrev = vCurr;
				}
				faceVCount++;
			}
		}
		TAutoPtr<CVertexBuffer> vb = new CVertexBuffer(vertBuffer.size() * sizeof(CVertexData), vertBuffer.data());
		Attribs.emplace_back("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, vb, 0, (uint32_t)sizeof(CVertexData));
		Attribs.emplace_back("NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, vb, 12, (uint32_t)sizeof(CVertexData));
		ElementCount = (uint32_t)vertBuffer.size();
	}
}

CStaticMeshGeometry::CAttribute* CStaticMeshGeometry::GetAttribute(const std::string& name, unsigned int index)
{
    //TODO: O(n) is suboptimal
    for (auto& attr : Attribs)
    {
        if (attr.Name == name && attr.Index == index)
            return &attr;
    }
    return nullptr;
}

TAutoPtr<CIndexBuffer> CStaticMeshGeometry::GetIndexBuffer() const
{
    return IB;
}

uint32_t CStaticMeshGeometry::GetElementCount() const
{
    return ElementCount;
}

D3D11_PRIMITIVE_TOPOLOGY CStaticMeshGeometry::GetPrimitiveTopology() const
{
	return PrimitiveTopology;
}

void CStaticMeshGeometry::SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY value)
{
	PrimitiveTopology = value;
}

}
