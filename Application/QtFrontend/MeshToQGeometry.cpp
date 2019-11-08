#include "MeshToQGeometry.h"

#include <Qt3DRender/QBuffer>

namespace Nome
{

CMeshToQGeometry::CMeshToQGeometry(const CMeshImpl& fromMesh, bool bGenPointGeometry)
{
    // Per face normal, thus no shared vertices between faces
    struct CVertexData
    {
        std::array<float, 3> Pos;
        std::array<float, 3> Normal;

        void SendToBuilder(CGeometryBuilder& builder) const
        {
            builder.Ingest(Pos[0], Pos[1], Pos[2]);
            builder.Ingest(Normal[0], Normal[1], Normal[2]);
        }
    };
    const uint32_t stride = sizeof(CVertexData);
    static_assert(stride == 24, "Vertex data size isn't as expected");
    QByteArray bufferArray;
    CAttribute attrPos { bufferArray, offsetof(CVertexData, Pos), stride,
                         Qt3DRender::QAttribute::Float, 3 };
    CAttribute attrNor { bufferArray, offsetof(CVertexData, Normal), stride,
                         Qt3DRender::QAttribute::Float, 3 };
    CGeometryBuilder builder;
    builder.AddAttribute(&attrPos);
    builder.AddAttribute(&attrNor);

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
                v0.SendToBuilder(builder);
                vPrev.SendToBuilder(builder);
                vCurr.SendToBuilder(builder);
                vPrev = vCurr;
            }
            faceVCount++;
        }
    }

    Geometry = new Qt3DRender::QGeometry();

    auto* buffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::VertexBuffer, Geometry);
    buffer->setData(bufferArray);

    auto* posAttr = new Qt3DRender::QAttribute(Geometry);
    posAttr->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());
    posAttr->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    posAttr->setBuffer(buffer);
    posAttr->setCount(builder.GetVertexCount());
    attrPos.FillInQAttribute(posAttr);
    Geometry->addAttribute(posAttr);

    auto* normAttr = new Qt3DRender::QAttribute(Geometry);
    normAttr->setName(Qt3DRender::QAttribute::defaultNormalAttributeName());
    normAttr->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    normAttr->setBuffer(buffer);
    normAttr->setCount(builder.GetVertexCount());
    attrNor.FillInQAttribute(normAttr);
    Geometry->addAttribute(normAttr);

    if (bGenPointGeometry)
    {
        PointGeometry = new Qt3DRender::QGeometry();

        std::vector<float> pointBufferData;
        uint32_t vertexCount = 0;
        for (const auto& v : fromMesh.vertices())
        {
            const auto& point = fromMesh.point(v);
            pointBufferData.push_back(point[0]);
            pointBufferData.push_back(point[1]);
            pointBufferData.push_back(point[2]);
            // TODO: if selected, change color to something else
            pointBufferData.push_back(1.0f);
            pointBufferData.push_back(1.0f);
            pointBufferData.push_back(1.0f);
            vertexCount++;
        }

        QByteArray copyOfBuffer { reinterpret_cast<const char*>(pointBufferData.data()), static_cast<int>(pointBufferData.size()) };
        auto* buffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::VertexBuffer, PointGeometry);
        buffer->setData(copyOfBuffer);

        posAttr = new Qt3DRender::QAttribute(PointGeometry);
        posAttr->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());
        posAttr->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
        posAttr->setBuffer(buffer);
        posAttr->setCount(vertexCount);
        posAttr->setByteOffset(0);
        posAttr->setByteStride(24);
        posAttr->setVertexBaseType(Qt3DRender::QAttribute::Float);
        posAttr->setVertexSize(3);
        PointGeometry->addAttribute(posAttr);

        auto* colorAttr = new Qt3DRender::QAttribute(PointGeometry);
        colorAttr->setName(Qt3DRender::QAttribute::defaultColorAttributeName());
        colorAttr->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
        colorAttr->setBuffer(buffer);
        colorAttr->setCount(vertexCount);
        colorAttr->setByteOffset(12);
        colorAttr->setByteStride(24);
        colorAttr->setVertexBaseType(Qt3DRender::QAttribute::Float);
        colorAttr->setVertexSize(3);
        PointGeometry->addAttribute(colorAttr);
    }
}

CMeshToQGeometry::~CMeshToQGeometry()
{
    if (!Geometry->parent())
        delete Geometry;
    if (PointGeometry && !PointGeometry->parent())
        delete PointGeometry;
}

}
