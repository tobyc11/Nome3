#pragma once

#define _USE_MATH_DEFINES
#undef min
#undef max
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>

#include <QByteArray>
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QGeometry>
#include <vector>

namespace Nome
{

typedef OpenMesh::PolyMesh_ArrayKernelT<> CMeshImpl;

class CAttribute
{
public:
    QByteArray& Buffer;
    uint32_t ByteOffset;
    uint32_t ByteStride;
    Qt3DRender::QAttribute::VertexBaseType Type;
    uint32_t Size;

    CAttribute(QByteArray& buffer, uint32_t byteOffset, uint32_t byteStride,
               Qt3DRender::QAttribute::VertexBaseType type, uint32_t size)
        : Buffer(buffer)
        , ByteOffset(byteOffset)
        , ByteStride(byteStride)
        , Type(type)
        , Size(size)
    {
    }

    [[nodiscard]] int GetAbsByteOffset(uint32_t i) const { return ByteOffset + ByteStride * i; }

    void FillInQAttribute(Qt3DRender::QAttribute* attr) const
    {
        attr->setByteOffset(ByteOffset);
        attr->setByteStride(ByteStride);
        attr->setVertexBaseType(Type);
        attr->setVertexSize(Size);
    }
};

class CGeometryBuilder
{
public:
    CGeometryBuilder() = default;

    void AddAttribute(const CAttribute* attr) { Attributes.push_back(attr); }

    void Ingest(float x, float y, float z)
    {
        auto* target = Attributes[CurrAttrIndex];

        // Type check for target
        assert(target->Type == Qt3DRender::QAttribute::Float);
        assert(target->Size == 3);

        // Append the buffer pointed to by target
        const int byteLen = sizeof(float) * 3;
        int begOffset = target->GetAbsByteOffset(CurrVertexIndex);
        if (target->Buffer.size() < begOffset + byteLen)
            target->Buffer.resize(begOffset + byteLen);
        auto* beg = &*target->Buffer.begin() + begOffset;
        auto* p = reinterpret_cast<float*>(beg);
        *p++ = x;
        *p++ = y;
        *p = z;

        AdvanceIndices();
    }

    uint32_t GetVertexCount() const { return CurrVertexIndex; }

protected:
    void AdvanceIndices()
    {
        CurrAttrIndex++;
        if (CurrAttrIndex == Attributes.size())
        {
            CurrAttrIndex = 0;
            CurrVertexIndex++;
        }
    }

private:
    std::vector<const CAttribute*> Attributes;
    uint32_t CurrVertexIndex = 0;
    uint32_t CurrAttrIndex = 0;
};

class CMeshToQGeometry
{
public:
    explicit CMeshToQGeometry(const CMeshImpl& fromMesh, bool bGenPointGeometry = false);

    ~CMeshToQGeometry();

    CMeshToQGeometry(const CMeshToQGeometry&) = delete;
    CMeshToQGeometry(CMeshToQGeometry&&) = delete;
    CMeshToQGeometry& operator=(const CMeshToQGeometry&) = delete;
    CMeshToQGeometry& operator=(CMeshToQGeometry&&) = delete;

    [[nodiscard]] Qt3DRender::QGeometry* GetGeometry() const { return Geometry; }
    [[nodiscard]] Qt3DRender::QGeometry* GetPointGeometry() const { return PointGeometry; }

private:
    Qt3DRender::QGeometry* Geometry = nullptr;
    Qt3DRender::QGeometry* PointGeometry = nullptr;
};

}
