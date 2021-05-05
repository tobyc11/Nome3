#pragma once
#include "Mesh.h"

namespace Nome::Scene
{

class CGenParametricSurf : public CMesh
{
    DEFINE_INPUT(float, u_start) { MarkDirty(); }
    DEFINE_INPUT(float, u_end) { MarkDirty(); }
    DEFINE_INPUT(float, v_start) { MarkDirty(); }
    DEFINE_INPUT(float, v_end) { MarkDirty(); }
    DEFINE_INPUT(float, u_segs) { MarkDirty(); }
    DEFINE_INPUT(float, v_segs) { MarkDirty(); }

public:
    DECLARE_META_CLASS(CGenParametricSurf, CMesh);
    CGenParametricSurf() = default;
    CGenParametricSurf(const std::string& name, const std::string& func)
        : CMesh(std::move(name))
        , Func(std::move(func))
    {
    }

    const std::string& GetFunc() const { return Func; }
    void UpdateEntity() override;


private:
    //..
    std::string Func; 
};

}
