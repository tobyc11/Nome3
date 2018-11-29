#pragma once

#include "Entity.h"

namespace Nome::Scene
{

class CPoint : public CEntity
{
public:
    CPoint(CDocument* doc, std::string name) : CEntity(doc, std::move(name))
    {
    }

    void SetDefaultPosition(float x, float y, float z);

    DEFINE_INPUT(float, X) { MarkDirty(); }
    DEFINE_INPUT(float, Y) { MarkDirty(); }
    DEFINE_INPUT(float, Z) { MarkDirty(); }

    DEFINE_OUTPUT_WITH_UPDATE(Vector3, Position)
    {
        Position.UpdateValue({X.GetValue(DefaultX), Y.GetValue(DefaultY), Z.GetValue(DefaultZ)});
    }

private:
    //Position used when no input is connected
    float DefaultX = 0.0f, DefaultY = 0.0f, DefaultZ = 0.0f;
};

}
