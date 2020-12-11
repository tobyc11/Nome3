#include "MaterialEnumsGL.h"
#include <stdexcept>

GLenum TrPrimitiveTopology(EPrimitiveTopology value)
{
    switch (value)
    {
    case EPrimitiveTopology::PointList:
        return GL_POINTS;
    case EPrimitiveTopology::LineList:
        return GL_LINES;
    case EPrimitiveTopology::LineStrip:
        return GL_LINE_STRIP;
    case EPrimitiveTopology::TriangleList:
        return GL_TRIANGLES;
    case EPrimitiveTopology::TriangleStrip:
        return GL_TRIANGLE_STRIP;
    case EPrimitiveTopology::TriangleFan:
        return GL_TRIANGLE_FAN;
    case EPrimitiveTopology::LineListWithAdjacency:
        return GL_LINES_ADJACENCY;
    case EPrimitiveTopology::LineStripWithAdjacency:
        return GL_LINE_STRIP_ADJACENCY;
    case EPrimitiveTopology::TriangleListWithAdjacency:
        return GL_TRIANGLES_ADJACENCY;
    case EPrimitiveTopology::TriangleStripWithAdjacency:
        return GL_TRIANGLE_STRIP_ADJACENCY;
    case EPrimitiveTopology::PatchList:
        return GL_PATCHES;
    }
    throw std::invalid_argument("Bad EPrimitiveTopology value");
}

GLenum TrCompareOp(ECompareOp value)
{
    switch (value)
    {
    case ECompareOp::Never:
        return GL_NEVER;
    case ECompareOp::Less:
        return GL_LESS;
    case ECompareOp::Equal:
        return GL_EQUAL;
    case ECompareOp::LEqual:
        return GL_LEQUAL;
    case ECompareOp::Greater:
        return GL_GREATER;
    case ECompareOp::NotEqual:
        return GL_NOTEQUAL;
    case ECompareOp::GEqual:
        return GL_GEQUAL;
    case ECompareOp::Always:
        return GL_ALWAYS;
    }
    throw std::invalid_argument("Bad ECompareOp value");
}
