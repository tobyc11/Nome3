#include "Camera.h"

namespace Nome::Scene
{

void CCamera::CalculateProjMatrix() const
{
    if (!bProjMatrixDirty)
        return;

    ProjMatrix = Matrix4::ZERO;

    if (true /*!bIsOrthographic*/)
    {
        float h = 1.0f / tanf(FovY * tc::M_DEGTORAD_2);
        float w = h / AspectRatio;
        float q = FarClip / (NearClip - FarClip);
        float qn = NearClip * q;

        ProjMatrix[0][0] = w;
        ProjMatrix[1][1] = h;
        ProjMatrix[2][2] = q;
        ProjMatrix[2][3] = qn;
        ProjMatrix[3][2] = -1.0f;
    }
    else
    {
        throw std::runtime_error("Unimplemented feature.");
    }

    bProjMatrixDirty = false;
}

Frustum CCamera::GetFrustum() const
{
    const auto& viewMatrix = GetPrincipleTreeNode()->GetL2W();
    auto frustum = Frustum();
    frustum.Define(FovY, AspectRatio, 1.0f, NearClip, FarClip, viewMatrix);
    return frustum;
}

CSceneTreeNode* CCamera::GetPrincipleTreeNode() const
{
    for (auto treeNode : GetTreeNodes())
        return treeNode;
    return nullptr;
}

}
