#pragma once
#include "Mesh.h"
#include "SweepPath.h"
#include "Vector3.h"

namespace Nome::Scene
{

class CSweepMath
{
public:
    Vector3 crossProduct(Vector3 vectorA, Vector3 vectorB);
    float getAngle(Vector3 vectorA, Vector3 vectorB);
    Vector3 getPerpendicularVector(Vector3 vectorA, Vector3 vectorB);
    Vector3 vectorMultiplyMatrix(Vector3 vector, float matrix[3][3]);
    float calculateRotateAngle(Vector3 vectorA, Vector3 vectorB, Vector3 T);
    Vector3 getDefaultN(Vector3 T);
    bool isAtSameLine(Vector3 vectorA, Vector3 vectorB);
    bool isAtSameDirection(Vector3 vectorA, Vector3 vectorB);
};

class CSweep : public CMesh
{
    DEFINE_INPUT(CSweepPathInfo *, Path) { MarkDirty(); }
    DEFINE_INPUT(CSweepPathInfo *, CrossSection) { MarkDirty(); }
    DEFINE_INPUT(float, Azimuth) { MarkDirty(); }
    DEFINE_INPUT(float, Twist) { MarkDirty(); }

public:
    DECLARE_META_CLASS(CSweep, CMesh);

    CSweep() = default;
    explicit CSweep(const std::string& name)
            : CMesh(std::move(name))
    {
    }

    void UpdateEntity() override;

private:
    void drawCrossSection(std::vector<Vector3> crossSection, Vector3 center,
                          Vector3 T, Vector3 N, float rotateAngle, float angle,
                          Vector3 controlScale, int index, bool shouldReverse);
    void drawCap(std::vector<Vector3> crossSection, int crossIndex,
                 int faceIndex, bool shouldFlip);
    CSweepMath Math = CSweepMath();
    bool bStartCap = false;
    bool bEndCap = false;
    bool bMintorsion = false;
    bool bReverse = false;
};

}
