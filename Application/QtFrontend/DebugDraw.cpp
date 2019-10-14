#include "DebugDraw.h"
#include "MaterialParser.h"
#include "ResourceMgr.h"

namespace Nome
{

CDebugDraw::CDebugDraw(Qt3DCore::QNode* parent)
    : Qt3DCore::QNode(parent)
{
    LineGeometry = std::make_unique<Qt3DRender::QGeometry>(this);
    auto* lineGeometry = LineGeometry.get();

    PointBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::VertexBuffer, lineGeometry);
    LineBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::VertexBuffer, lineGeometry);

    auto* posAttr = new Qt3DRender::QAttribute(lineGeometry);
    posAttr->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());
    posAttr->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    posAttr->setBuffer(LineBuffer);
    posAttr->setCount(2); // Subject to updates
    posAttr->setByteOffset(0);
    posAttr->setByteStride(sizeof(float) * 6);
    posAttr->setVertexBaseType(Qt3DRender::QAttribute::Float);
    posAttr->setVertexSize(3);
    lineGeometry->addAttribute(posAttr);

    auto* colorAttr = new Qt3DRender::QAttribute(lineGeometry);
    colorAttr->setName(Qt3DRender::QAttribute::defaultColorAttributeName());
    colorAttr->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    colorAttr->setBuffer(LineBuffer);
    colorAttr->setCount(2); // Subject to updates
    colorAttr->setByteOffset(sizeof(float) * 3);
    colorAttr->setByteStride(sizeof(float) * 6);
    colorAttr->setVertexBaseType(Qt3DRender::QAttribute::Float);
    colorAttr->setVertexSize(3);
    lineGeometry->addAttribute(colorAttr);
}

void CDebugDraw::Reset()
{
    PointData.clear();
    LineData.clear();
}

void CDebugDraw::Commit()
{
    LineBuffer->setData(LineData);

    // Calculate primitive counts from buffer lengths
    unsigned int lineVertCount = LineData.size() / (sizeof(float) * 6);
    LineData.clear();

    // Locate the attributes and change their counts
    for (auto* attr : LineGeometry->attributes())
    {
        attr->setCount(lineVertCount);
    }
}

void CDebugDraw::DrawPoint(tc::Vector3 pos, tc::Color color)
{
    // Append appropriate data at the end of the point buffer:
    // VVVCCC/VVVCCC/VVVCCC/...

    int sizeInBytes = sizeof(float) * 6;
    int offset = PointData.size();
    // Resize the array and point p to the original end
    PointData.resize(PointData.size() + sizeInBytes);
    auto* p = reinterpret_cast<float*>(PointData.data() + offset);
    *p++ = pos.x;
    *p++ = pos.y;
    *p++ = pos.z;
    *p++ = color.r;
    *p++ = color.g;
    *p++ = color.b;
}

void CDebugDraw::LineSegment(tc::Vector3 from, tc::Color fromColor, tc::Vector3 to,
                             tc::Color toColor)
{
    // VVVCCCx2/VVVCCCx2
    int sizeInBytes = sizeof(float) * 6 * 2;
    int offset = LineData.size();
    LineData.resize(LineData.size() + sizeInBytes);
    auto* p = reinterpret_cast<float*>(LineData.data() + offset);
    *p++ = from.x;
    *p++ = from.y;
    *p++ = from.z;
    *p++ = fromColor.r;
    *p++ = fromColor.g;
    *p++ = fromColor.b;
    *p++ = to.x;
    *p++ = to.y;
    *p++ = to.z;
    *p++ = toColor.r;
    *p++ = toColor.g;
    *p++ = toColor.b;
}

}
