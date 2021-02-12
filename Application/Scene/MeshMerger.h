#pragma once
#include "Mesh.h"
#include <LangUtils.h>
#include <OpenMesh/Tools/Subdivider/Uniform/CatmullClarkT.hh>
namespace Nome::Scene
{

class CMeshMerger : public CMesh
{
public:
    using Super = CMesh;
    CMeshMerger() = default;
    CMeshMerger(const std::string& name)
        : CMesh(std::move(name))
    {
    }

    void UpdateEntity() override;

    // No update yet, please just use one time
    void MergeIn(const CMeshInstance& meshInstance);

    // Buggy, preset to 2 subdivision steps
    void Catmull(const CMeshInstance& meshInstance);

    // hacky way to fix Mobius face, where CCW goes to CW edge orientation
    std::vector<Vector3> fixEdgeOrientation(std::vector<Vector3> facePositions);

private:
    std::pair<Vertex*, float> FindClosestVertex(const tc::Vector3& pos);

    unsigned int VertCount = 0;
    unsigned int FaceCount = 0;
};

}