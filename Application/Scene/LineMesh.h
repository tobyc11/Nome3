#pragma once
#include "Mesh.h"

namespace Nome::Scene
{

class CLineMesh : public CMesh
{
    DEFINE_INPUT(float, input1) { MarkDirty(); }
//    DEFINE_INPUT_ARRAY(char*, input1) { MarkDirty(); }

public:
    DECLARE_META_CLASS(CLineMesh, CMesh);
    CLineMesh() = default;
    CLineMesh(const std::string& name)
        : CMesh(std::move(name))
    {
    }

    void UpdateEntity() override;


private:
    //..
};

}
