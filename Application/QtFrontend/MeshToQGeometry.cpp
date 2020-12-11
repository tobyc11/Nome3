#include "MeshToQGeometry.h"
#include <Qt3DRender/QBuffer>
#include <iostream>
namespace Nome
{

CMeshToQGeometry::CMeshToQGeometry(const CMeshImpl& fromMesh,
                                   std::vector<CMeshImpl::FaceHandle> selectedFaceHandles,
                                   bool bGenPointGeometry)
{
    // Per face normal, thus no shared vertices between faces
    struct CVertexData
    {
        std::array<float, 3> Pos; // a float is 4 bytes
        std::array<float, 3> Normal;
        int colorSelected; // Randy added this to color faces that are selected

        void SendToBuilder(
            CGeometryBuilder& builder) const // Randy note: I think here it's just decomposing it
                                             // into a list of triangles with the correct normals
        {
            builder.Ingest(Pos[0], Pos[1], Pos[2]);
            builder.Ingest(Normal[0], Normal[1], Normal[2]);
            builder.IngestInt(colorSelected);
        }
    };
    const uint32_t stride = sizeof(CVertexData);
    static_assert(stride == 28, "Vertex data size isn't as expected");
    QByteArray bufferArray;
    CAttribute attrPos { bufferArray, offsetof(CVertexData, Pos), stride,
                         Qt3DRender::QAttribute::Float, 3 };
    CAttribute attrNor { bufferArray, offsetof(CVertexData, Normal), stride,
                         Qt3DRender::QAttribute::Float, 3 };
    CAttribute attrcolorSelected { bufferArray, offsetof(CVertexData, colorSelected), stride,
                                   Qt3DRender::QAttribute::Int, 1 };
    CGeometryBuilder builder;
    builder.AddAttribute(&attrPos);
    builder.AddAttribute(&attrNor);
    builder.AddAttribute(&attrcolorSelected);

    CMeshImpl::FaceIter fIter, fEnd = fromMesh.faces_end();
    for (fIter = fromMesh.faces_sbegin(); fIter != fEnd; ++fIter)
    {
        CVertexData v0, vPrev, vCurr;
        int faceVCount = 0;
        CMeshImpl::FaceVertexIter fvIter = CMeshImpl::FaceVertexIter(fromMesh, *fIter);

        // Check to see if this face handle was selected
        int selected = 0; // 0 means it hasn't been selected and it will retain original color
        auto iter = std::find(selectedFaceHandles.begin(), selectedFaceHandles.end(), fIter);
        if (iter != selectedFaceHandles.end())
        {
            selected = 1;
        }
        for (; fvIter.is_valid(); ++fvIter)
        {
            CMeshImpl::VertexHandle faceVert = *fvIter;
            const auto& posVec = fromMesh.point(faceVert);
            if (faceVCount == 0) // first vert of triangle
            {
                const auto& posVec = fromMesh.point(faceVert);
                v0.Pos = { posVec[0], posVec[1], posVec[2] };
                const auto& fnVec = fromMesh.normal(*fIter);
                v0.Normal = { fnVec[0], fnVec[1], fnVec[2] };
                v0.colorSelected =
                    selected; // Randy added this to handle marking which things are selected.
            }
            else if (faceVCount == 1) // second vert of triangle
            {
                const auto& posVec = fromMesh.point(faceVert);
                vPrev.Pos = { posVec[0], posVec[1], posVec[2] };
                const auto& fnVec = fromMesh.normal(*fIter);
                vPrev.Normal = { fnVec[0], fnVec[1], fnVec[2] };
                vPrev.colorSelected = selected;
            }
            else // third vert of triangle
            {
                const auto& posVec = fromMesh.point(faceVert);
                vCurr.Pos = { posVec[0], posVec[1], posVec[2] };
                const auto& fnVec = fromMesh.normal(*fIter);
                vCurr.Normal = { fnVec[0], fnVec[1], fnVec[2] };
                vCurr.colorSelected = selected;
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
    posAttr->setName(
        Qt3DRender::QAttribute::defaultPositionAttributeName()); // default is vertexPosition. This
                                                                 // is used as input in the .vert
                                                                 // shader
    posAttr->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    posAttr->setBuffer(buffer);
    posAttr->setCount(builder.GetVertexCount());
    attrPos.FillInQAttribute(posAttr);
    Geometry->addAttribute(posAttr);

    auto* normAttr = new Qt3DRender::QAttribute(Geometry);
    std::cout << Qt3DRender::QAttribute::defaultNormalAttributeName().toStdString() << std::endl;
    normAttr->setName(
        Qt3DRender::QAttribute::defaultNormalAttributeName()); // default is vertexNormal. This is
                                                               // used as input in the .vert shader
    normAttr->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    normAttr->setBuffer(buffer);
    normAttr->setCount(builder.GetVertexCount());
    attrNor.FillInQAttribute(normAttr);
    Geometry->addAttribute(normAttr);

    // Randy added this to handle marking which things are selected.  Not sure if needed
    auto* colorSelectedAttr = new Qt3DRender::QAttribute(Geometry);
    colorSelectedAttr->setName(QString::fromStdString("colorSelected"));
    colorSelectedAttr->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    colorSelectedAttr->setBuffer(buffer);
    colorSelectedAttr->setCount(builder.GetVertexCount());
    attrcolorSelected.FillInQAttribute(colorSelectedAttr);
    Geometry->addAttribute(colorSelectedAttr);

    if (bGenPointGeometry)
    {
        PointGeometry = new Qt3DRender::QGeometry();

        std::vector<float> pointBufferData;
        uint32_t vertexCount = 0;

        // this is how the vertex is displayed
        for (const auto& v : fromMesh.vertices())
        {
            const auto& point = fromMesh.point(v);
            const auto& color = fromMesh.color(v);
            pointBufferData.push_back(point[0]);
            pointBufferData.push_back(point[1]);
            pointBufferData.push_back(point[2]);
            // TODO: if selected, change color to something else
            pointBufferData.push_back(color[0] / 255.0f);
            pointBufferData.push_back(color[1] / 255.0f);
            pointBufferData.push_back(color[2] / 255.0f);
            // printf("v%d: %d %d %d\n", vertexCount, color[0], color[1], color[2]); // Randy
            // commented this out on 11/26. Printing coordinates more useful

            printf("v%d: %f %f %f\n", v.idx(), point[0], point[1], point[2]);
            vertexCount++;
        }
        QByteArray copyOfBuffer { reinterpret_cast<const char*>(pointBufferData.data()),
                                  static_cast<int>(pointBufferData.size() * sizeof(float)) };
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