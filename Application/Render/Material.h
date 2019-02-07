#pragma once
#include "IncludeD3D11.h"
#include <Vector3.h>
#include <AutoPtr.h>

namespace Nome
{

using tc::Vector3;

//Minimally featured. Extend later when we have time.
class CMaterial : public tc::FRefCounted
{
public:
    Vector3 GetColor() const { return Color; }
    void SetColor(const Vector3& value) { Color = value; }

    bool IsWireframe() const;
    void SetWireframe(bool value);

    void Bind(ID3D11DeviceContext* ctx);

private:
    Vector3 Color = Vector3::ONE;
    bool bIsWireframe;

    //Pipeline states
    bool bStatesDirty = true;
    ComPtr<ID3D11RasterizerState> Rast;
    ComPtr<ID3D11DepthStencilState> DepthStencil;
    ComPtr<ID3D11BlendState> Blend;
};

}
