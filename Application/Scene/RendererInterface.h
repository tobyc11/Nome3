#pragma once

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

}
