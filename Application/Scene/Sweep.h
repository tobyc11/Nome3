#pragma once
#include "Mesh.h"
#include "SweepPath.h"
#include "Vector3.h"

namespace Nome::Scene
{

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
    bool bStartCap = false;
    bool bEndCap = false;
    bool bMintorsion = false;
    bool bReverse = false;
};

}
