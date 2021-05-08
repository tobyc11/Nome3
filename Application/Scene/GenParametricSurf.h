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
    CGenParametricSurf(const std::string& name, const std::string& funcX, const std::string& funcY, const std::string& funcZ)
        : CMesh(std::move(name))
        , FuncX(std::move(funcX))
        , FuncY(std::move(funcY))
        , FuncZ(std::move(funcZ))
    {
    }

    const std::string& GetFuncX() const { return FuncX; }
    const std::string& GetFuncY() const { return FuncY; }
    const std::string& GetFuncZ() const { return FuncZ; }
    void UpdateEntity() override;


private:
    //..
    std::string FuncX;
    std::string FuncY;
    std::string FuncZ;
};

}
