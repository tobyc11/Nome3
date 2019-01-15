#include "Camera.h"

#include "imgui.h"

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

const Matrix4& CCamera::GetProjMatrix() const
{
    CalculateProjMatrix();
    return ProjMatrix;
}

Matrix4 CCamera::GetViewMatrix() const
{
	if (SceneTreeNode)
	{
		auto viewInv = SceneTreeNode->L2WTransform.GetValue(Matrix3x4::IDENTITY);
		return viewInv.Inverse().ToMatrix4();
	}
	return Matrix4::IDENTITY;
}

Frustum CCamera::GetFrustum() const
{
	auto frustum = Frustum();
	if (SceneTreeNode)
	{
		const auto& viewMatrix = SceneTreeNode->L2WTransform.GetValue(Matrix3x4::IDENTITY);

		frustum.Define(FovY, AspectRatio, 1.0f, NearClip, FarClip, viewMatrix);
		return frustum;
	}

	return frustum;
}

void CCamera::ShowDebugImGui()
{
}

bool COrbitCameraController::MouseMoved(const CMouseState& state)
{
	if (bIsMouseDown)
	{
		//auto currentTransform = Subject->GetTransform();
		auto rotMat = Matrix3x4(Quaternion(-1.0f * state.dx, Vector3::UP).RotationMatrix());
		//Subject->SetDefaultTransform(rotMat * currentTransform);
		return true;
	}
	return false;
}

bool COrbitCameraController::MouseButtonPressed(int index)
{
	if (index == 0 || index == 2)
	{
		bIsMouseDown = true;
		return true;
	}
	return false;
}

bool COrbitCameraController::MouseButtonReleased(int index)
{
	if (index == 0 || index == 2)
	{
		bIsMouseDown = false;
		return true;
	}
	return false;
}

}
