#pragma once
#include "Entity.h"
#include <Flow/FlowNodeArray.h>
#include "ControlPoint.h"

namespace Nome::Scene
{



class CSharp : public CEntity
{


DEFINE_INPUT_ARRAY(CControlPointInfo*, ControlPoints) { MarkDirty(); }

DEFINE_OUTPUT_WITH_UPDATE(CSharp*, SharpPoints) { UpdateEntity(); }

    void MarkDirty() override;
    void UpdateEntity() override;

public:
    DECLARE_META_CLASS(CSharp, CEntity);

    CPoint() = default;

    explicit CPoint(std::string name)
        : CEntity(std::move(name))
    {
    }

private:
    CVertexInfo VI;

};

}
