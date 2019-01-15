#pragma once
#include <AutoPtr.h>
#include <Vector3.h>

#include <d3d11_1.h>
#include <wrl/client.h>

#define _USE_MATH_DEFINES
#undef min
#undef max
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>

#include <string>
#include <vector>

typedef OpenMesh::PolyMesh_ArrayKernelT<> CNomeMesh;

namespace Nome
{

using tc::Vector3;

using Microsoft::WRL::ComPtr;
using tc::TAutoPtr;

class CVertexBuffer : public tc::FRefCounted
{
public:
	CVertexBuffer(size_t size);
	CVertexBuffer(size_t size, void* data);
	~CVertexBuffer();

	bool IsOnCPU() const { return CPUBuffer; }
	bool IsOnGPU() const;
	void UploadToGPU();

private:
	size_t CPUSize = 0;
	void* CPUBuffer = nullptr;
	ComPtr<ID3D11Buffer> GPUBuffer;
};

class CIndexBuffer : public tc::FRefCounted
{
public:
	CIndexBuffer(size_t size, size_t elementWidth, void* data);
	
private:
	size_t ElementWidth;
	ComPtr<ID3D11Buffer> GPUBuffer;
};

class CGeometry : public tc::FRefCounted
{
public:
};

class CStaticMeshGeometry : public CGeometry
{
public:
	struct CAttribute
	{
		CAttribute(const std::string& name, uint32_t index, DXGI_FORMAT format, CVertexBuffer* buffer, uint32_t offset, uint32_t stride);

		std::string Name;
		uint32_t Index;

		DXGI_FORMAT Format;

		//Buffer view
		TAutoPtr<CVertexBuffer> Buffer;
		uint32_t Offset;
		uint32_t Stride;
	};

	CStaticMeshGeometry();
	CStaticMeshGeometry(const CNomeMesh& fromMesh);

	void Reconstruct(const CNomeMesh& fromMesh);

private:
	std::vector<CAttribute> Attribs;
	TAutoPtr<CIndexBuffer> IB;
};

}
