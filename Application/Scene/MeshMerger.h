#pragma once
#include "Mesh.h"
#include <LangUtils.h>
#include <opensubdiv/far/topologyRefinerFactory.h>
#include <opensubdiv/far/primvarRefiner.h>

#include <cstdio>

namespace Nome::Scene
{

class CMeshMerger : public CMesh
{
    DEFINE_INPUT(float, Level) { MarkDirty(); }

    void UpdateEntity() override;
    //void MarkDirty() override;

public:
    DECLARE_META_CLASS(CMeshMerger, CEntity);

    CMeshMerger() = default;
    CMeshMerger(const std::string& name)
        : CMesh(std::move(name))
    {
    }


    // No update yet, please just use one time
    void MergeIn(CMeshInstance& meshInstance, bool markedSharp = false);

    void MergeClear();

    // Buggy, preset to 3 subdivision steps
    void Catmull();

    bool subdivide(CMeshImpl& _m, int n, const bool _update_points);

    void split_face(CMeshImpl& _m, const CMeshImpl::FaceHandle& _fh);

    void split_edge(CMeshImpl& _m, const CMeshImpl::EdgeHandle& _eh);

    void compute_midpoint(CMeshImpl& _m, const CMeshImpl::EdgeHandle& _eh, const bool _update_points);

    void update_vertex( CMeshImpl& _m, const CMeshImpl::VertexHandle& _vh);

    bool prepare(CMeshImpl& _m);

    bool cleanup(CMeshImpl& _m);

    void setSubLevel(int level) {
        subdivisionLevel = level;
    }


private:
    std::pair<CMeshImpl::VertexHandle, float> FindClosestVertex(const tc::Vector3& pos);

    unsigned int VertCount = 0;
    unsigned int FaceCount = 0;

    OpenMesh::VPropHandleT< CMeshImpl::Point > vp_pos_; // next vertex pos
    OpenMesh::EPropHandleT< CMeshImpl::Point > ep_pos_; // new edge pts
    OpenMesh::FPropHandleT< CMeshImpl::Point > fp_pos_; // new face pts
    OpenMesh::EPropHandleT<double> creaseWeights_; // crease weights

    CMeshImpl MergedMesh;
    unsigned int subdivisionLevel = 0;


};

}