#include "Transforms.h"

namespace Nome::Scene
{

void CTranslate::RecomputeOutput()
{
    Matrix3x4 prev = Input.GetValue(Matrix3x4::IDENTITY);
    prev.m03_ += X.GetValue(0.0f);
    prev.m13_ += Y.GetValue(0.0f);
    prev.m23_ += Z.GetValue(0.0f);
    Output.UpdateValue(prev);
}

void CRotate::RecomputeOutput()
{
    Matrix3x4 prev = Input.GetValue(Matrix3x4::IDENTITY);
    Quaternion rot = Quaternion(
        Angle.GetValue(0.0f), { AxisX.GetValue(0.0f), AxisY.GetValue(0.0f), AxisZ.GetValue(0.0f) });
    Matrix3x4 rotMat;
    rotMat.SetRotation(rot.RotationMatrix());
    Output.UpdateValue(rotMat * prev);
}

void CScale::RecomputeOutput()
{
    Matrix3x4 prev = Input.GetValue(Matrix3x4::IDENTITY);
    Matrix3x4 scale;
    scale.SetScale({ X.GetValue(1.0f), Y.GetValue(1.0f), Z.GetValue(1.0f) });
    Output.UpdateValue(scale * prev);
}

}
