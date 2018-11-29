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
        float h = 1.0f / tanf(FovY / 2);
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

}
