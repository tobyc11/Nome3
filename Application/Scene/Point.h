#pragma once
#include "Entity.h"

namespace Nome::Scene
{

struct CVertexInfo
{
    Vector3 Position;
    std::string Name;

    float sharpness = 0;
    virtual ~CVertexInfo() = default;


};

class CPoint : public CEntity
{
    DEFINE_INPUT(float, X) { MarkDirty(); }
    DEFINE_INPUT(float, Y) { MarkDirty(); }
    DEFINE_INPUT(float, Z) { MarkDirty(); }


    DEFINE_OUTPUT_WITH_UPDATE(CVertexInfo*, Point) { UpdateEntity(); }

    void MarkDirty() override;
    void UpdateEntity() override;

public:
    DECLARE_META_CLASS(CPoint, CEntity);

    CPoint() = default;

    explicit CPoint(std::string name)
        : CEntity(std::move(name))
    {
    }

private:
    CVertexInfo VI;

};

}
