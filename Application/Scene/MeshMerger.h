#pragma once
#include "Mesh.h"
#include <LangUtils.h>

namespace Nome::Scene
{

class CMeshMerger : public tc::FNoCopyMove
{
public:
    // No update yet, please just use one time
    void MergeIn(const CMeshInstance& meshInstance);
    TAutoPtr<CEntity> GetResultMesh();

private:
    std::pair<CMeshImpl::VertexHandle, float> FindClosestVertex(const tc::Vector3& pos);

    unsigned int VertCount = 0;
    unsigned int FaceCount = 0;

    // Exactly mirrors the corresponding data in Mesh
    CMeshImpl Mesh;
    std::map<std::string, CMeshImpl::VertexHandle> NameToVert;
    std::map<std::string, CMeshImpl::FaceHandle> NameToFace;
};

}
