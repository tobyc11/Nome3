#pragma once
#include "Mesh.h"
#include <LangUtils.h>

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
    explicit CMeshMerger(const std::string& name)
        : CMesh(name)
    {
    }


    // No update yet, please just use one time
    void MergeIn(CMeshInstance& meshInstance);

    void MergeClear();

    // sd_flag can be set to sharp and plain cc to have different types of subdivision
    void Catmull();

    bool subdivide(CMeshImpl& _m, unsigned int n, bool isSharp);

    void split_face(CMeshImpl& _m, const CMeshImpl::FaceHandle& _fh);

    void split_edge(CMeshImpl& _m, const CMeshImpl::EdgeHandle& _eh);

    void compute_midpoint(CMeshImpl& _m, const CMeshImpl::EdgeHandle& _eh, bool _update_points);

    void update_vertex( CMeshImpl& _m, const CMeshImpl::VertexHandle& _vh);

    void SetSharp(bool setSharp) {isSharp = setSharp;}


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

    CMeshImpl MergedMesh;
    unsigned int subdivisionLevel = 0;
    bool isSharp = true;


};


}




