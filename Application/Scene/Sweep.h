#pragma once
#include "Mesh.h"
#include "Polyline.h"
#include "Vector3.h"

namespace Nome::Scene
{

class CSweep : public CMesh
{
    DEFINE_INPUT(CPolylineInfo *, Path) { MarkDirty(); }
    DEFINE_INPUT(CPolylineInfo *, CrossSection) { MarkDirty(); }
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
    void drawCrossSection(std::vector<Vector3> crossSection, Vector3 center, Vector3 T, Vector3 N,
                          float angle, float scale, int index);
};

}
