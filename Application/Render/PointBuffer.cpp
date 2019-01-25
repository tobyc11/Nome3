#include "PointBuffer.h"
#include "Renderer.h"
#include "GraphicsDevice.h"

namespace Nome
{

class CPointBufferImpl
{
public:
    CPointBufferImpl(uint32_t size) : Size(size)
    {
        CPUBuffer.reserve(size);
        FreeList.reserve(size);
        for (uint32_t i = 0; i < size; i++)
            FreeList[i] = i + 1;

        D3D11_BUFFER_DESC desc;
        desc.ByteWidth = size * sizeof(Vector3);
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;
        desc.StructureByteStride = 0;
        GRenderer->GetGD()->GetDevice()->CreateBuffer(&desc, nullptr, GPUBuffer.GetAddressOf());
    }

    uint32_t Alloc()
    {
        uint32_t result = FirstFree;
        FirstFree = FreeList[FirstFree];
        Used++;
        return result;
    }

    void Free(uint32_t index)
    {
        FreeList[index] = FirstFree;
        FirstFree = index;
        Used--;
    }

    uint32_t GetSize() const
    {
        return Size;
    }

    void SetValue(uint32_t index, const Vector3& value)
    {
        CPUBuffer[index] = value;
    }

    void UpdateGPUBuffer(ID3D11DeviceContext* ctx)
    {
        D3D11_MAPPED_SUBRESOURCE resource;
        ctx->Map(GPUBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
        memcpy(resource.pData, CPUBuffer.data(), CPUBuffer.size() * sizeof(Vector3));
        ctx->Unmap(GPUBuffer.Get(), 0);
    }

private:
    ComPtr<ID3D11Buffer> GPUBuffer;
    std::vector<Vector3> CPUBuffer;

    std::vector<uint32_t> FreeList;
    uint32_t FirstFree = 0;
    uint32_t Used = 0, Size;
};

CPointBuffer::CPointBuffer()
{
}

}
