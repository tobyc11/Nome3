#pragma once
#include <Color.h>
#include <Vector3.h>

namespace Nome
{

class IDebugDraw
{
public:
    ///Use default instance color
    virtual void DrawPoint(tc::Vector3 pos) = 0;
    virtual void LineSegment(tc::Vector3 from, tc::Vector3 to) = 0;

    virtual void DrawPoint(tc::Vector3 pos, tc::Color color) = 0;
    virtual void LineSegment(tc::Vector3 from, tc::Color fromColor, tc::Vector3 to,
                             tc::Color toColor) = 0;

protected:
    ~IDebugDraw() = default;
};

}
