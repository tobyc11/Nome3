#pragma once
#include <Vector3.h>
#include <Color.h>

namespace Nome
{

class IMeshRenderer
{
public:
    enum ERenderFlags {
        HasFaceColor,
        DrawWireframe,
        DrawPoints,
    };

    virtual int GetRenderFlags() = 0;

    // Is this granular enough, or should CMesh handle more specific tasks
    virtual void NotifyGeometryChange() = 0;

protected:
    ~IMeshRenderer() = default;
};

class IDebugDraw
{
public:
    virtual void DrawPoint(tc::Vector3 pos, tc::Color color) = 0;
    virtual void LineSegment(tc::Vector3 from, tc::Color fromColor, tc::Vector3 to, tc::Color toColor) = 0;

protected:
    ~IDebugDraw() = default;
};

}
