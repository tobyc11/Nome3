#include "Camera.h"

namespace Nome::Scene
{

// We assume n, f are both positive where n < f. l, r, t, b can be either sign.
static Matrix4 ProjRightHandedDepthNegOneToOne(float l, float r, float t, float b, float n, float f)
{
    return Matrix4(2 * n / (r - l), 0, (r + l) / (r - l), 0,
                   0, 2 * n / (t - b), (t + b) / (t - b), 0,
                   0, 0, -(f + n) / (f - n), -2 * f * n / (f - n),
                   0, 0, -1, 0);
}

// TODO: use the proper test framework
static void TestProjMatrices()
{
    Matrix4 P = ProjRightHandedDepthNegOneToOne(-2.0f, 2.0f, 1.0f, -1.0f, 1.0f, 100.0f);
    auto topRight = P * Vector3(2.0f, 1.0f, -1.0f);
    assert(topRight.x == 1.0f);
    assert(topRight.y == 1.0f);
    assert(topRight.z <= -0.9f);
    topRight = P * Vector3(4.0f, 2.0f, -2.0f);
    assert(topRight.x == 1.0f);
    assert(topRight.y == 1.0f);
    assert(topRight.z >= -0.1f);
}

Matrix4 CLens::CalculateProjMatrix() const
{
    if (!bIsOrthographic)
    {
        float n = NearClip, f = FarClip;
        float t = n * tanf(FovY * tc::M_DEGTORAD_2);
        float r = t * AspectRatio;
        return ProjRightHandedDepthNegOneToOne(-r, r, t, -t, n ,f);
    }
    else
    {
        throw std::runtime_error("Unimplemented feature.");
    }
}

Matrix4 CCamera::GetViewMatrix() const
{
    auto view = Matrix3x4();
    // Make an orthonormal basis where -z points at the pivot, then transpose it
    auto z = (Position - Pivot).Normalized();
    auto x = Up.CrossProduct(z).Normalized();
    auto y = z.CrossProduct(x).Normalized();
    auto rotT = tc::Matrix3(x, y, z);
    view.SetRotation(rotT);
    view.SetTranslation(Position);
    return view.Inverse().ToMatrix4();
}

Frustum CCamera::GetFrustum() const
{
    auto frustum = Frustum();
    auto viewProj = CalculateProjMatrix() * GetViewMatrix();
    frustum.Define(viewProj);
    return frustum;
}

void CCamera::Orbit(CCamera::EDir dir, float angle)
{
    auto arrow = Position - Pivot;
    Vector3 axis;
    if (dir == EDir::Left || dir == EDir::Up)
        angle = -angle;
    if (dir == EDir::Left || dir == EDir::Right)
        axis = Up;
    else
    {
        axis = Up.CrossProduct(arrow).Normalized();
        // Prevent from going over +-90 degrees
        auto a = acosf(Up.Normalized().DotProduct(arrow.Normalized()));
        a += angle;
        if (a <= 0.0f || a >= tc::M_PI)
            return;
    }
    auto quat = Quaternion(angle, axis);
    auto newArrow = quat * arrow;
    Position = Pivot + newArrow;
}

void CCamera::Turn(CCamera::EDir dir, float angle)
{
    // Virtually the same code as Orbit, except the first and last line
    auto arrow = Pivot - Position;
    Vector3 axis;
    if (dir == EDir::Left || dir == EDir::Up)
        angle = -angle;
    if (dir == EDir::Left || dir == EDir::Right)
        axis = Up;
    else
    {
        axis = Up.CrossProduct(arrow).Normalized();
        // Prevent from going over +-90 degrees
        auto a = acosf(Up.Normalized().DotProduct(arrow.Normalized()));
        a += angle;
        if (a <= 0.0f || a >= tc::M_PI)
            return;
    }
    auto quat = Quaternion(angle, axis);
    auto newArrow = quat * arrow;
    Pivot = Position + newArrow;
}

void CCamera::Move(Vector3 deltaPos)
{
    Position += deltaPos;
    Pivot += deltaPos;
}

}
