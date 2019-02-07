#pragma once
#include <AutoPtr.h>
#include <Vector3.h>
#include "IncludeD3D11.h"

#define _USE_MATH_DEFINES
#undef min
#undef max
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>

#include <string>
#include <vector>

typedef OpenMesh::PolyMesh_ArrayKernelT<> CMeshImpl;

namespace Nome
{

using tc::Vector3;

using tc::TAutoPtr;

class CVertexBuffer : public tc::FRefCounted
{
public:
    CVertexBuffer(size_t size);
    CVertexBuffer(size_t size, const void* data);
    ~CVertexBuffer();

    bool IsOnCPU() const { return CPUBuffer; }
    bool IsOnGPU() const;
    void UploadToGPU();

    ID3D11Buffer* GetD3D11Buffer() const
    {
        return GPUBuffer.Get();
    }

private:
    size_t CPUSize = 0;
    void* CPUBuffer = nullptr;
    ComPtr<ID3D11Buffer> GPUBuffer;
};

class CIndexBuffer : public tc::FRefCounted
{
public:
    CIndexBuffer(size_t size, size_t elementWidth, void* data);

    void Bind(ID3D11DeviceContext* ctx)
    {
        DXGI_FORMAT format = DXGI_FORMAT_R16_UINT;
        if (ElementWidth == 4)
            format = DXGI_FORMAT_R32_UINT;
        else if (ElementWidth == 2)
            format = DXGI_FORMAT_R16_UINT;
        ctx->IASetIndexBuffer(GPUBuffer.Get(), format, 0);
    }
    
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
    CStaticMeshGeometry(const CMeshImpl& fromMesh);
    CStaticMeshGeometry(const std::vector<Vector3>& positions);

    void Reconstruct(const CMeshImpl& fromMesh);

    CAttribute* GetAttribute(const std::string& name, unsigned int index = 0);

    TAutoPtr<CIndexBuffer> GetIndexBuffer() const;

    uint32_t GetElementCount() const;

    D3D11_PRIMITIVE_TOPOLOGY GetPrimitiveTopology() const;
    void SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY value);

private:
    std::vector<CAttribute> Attribs;
    TAutoPtr<CIndexBuffer> IB;
    uint32_t ElementCount = 0;
    D3D11_PRIMITIVE_TOPOLOGY PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};

}
