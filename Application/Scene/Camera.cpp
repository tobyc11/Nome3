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
    const auto& viewMatrix = GetPrincipleTreeNode()->GetL2W();
    return viewMatrix.Inverse().ToMatrix4();
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

void CCamera::ShowDebugImGui()
{
	ImGui::Text("Camera:");
	ImGui::SameLine();
	if (ImGui::Button("<"))
	{
		auto currentTransform = GetTransform();
		auto rotMat = Matrix3x4(Quaternion(-10.0f, Vector3::UP).RotationMatrix());
		SetDefaultTransform(rotMat * currentTransform);
	}
	ImGui::SameLine();
	if (ImGui::Button(">"))
	{
		auto currentTransform = GetTransform();
		auto rotMat = Matrix3x4(Quaternion(10.0f, Vector3::UP).RotationMatrix());
		SetDefaultTransform(rotMat * currentTransform);
	}
	ImGui::SameLine();
	if (ImGui::Button("^"))
	{
		auto currentTransform = GetTransform();
		auto rotMat = Matrix3x4(Quaternion(-10.0f, Vector3::RIGHT).RotationMatrix());
		SetDefaultTransform(rotMat * currentTransform);
	}
	ImGui::SameLine();
	if (ImGui::Button("v"))
	{
		auto currentTransform = GetTransform();
		auto rotMat = Matrix3x4(Quaternion(10.0f, Vector3::RIGHT).RotationMatrix());
		SetDefaultTransform(rotMat * currentTransform);
	}
}

bool COrbitCameraController::MouseMoved(const CMouseState& state)
{
	if (bIsMouseDown)
	{
		auto currentTransform = Subject->GetTransform();
		auto rotMat = Matrix3x4(Quaternion(-1.0f * state.dx, Vector3::UP).RotationMatrix());
		Subject->SetDefaultTransform(rotMat * currentTransform);
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
