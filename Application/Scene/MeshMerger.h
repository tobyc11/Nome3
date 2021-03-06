#pragma once
#include "Mesh.h"
#include <LangUtils.h>

#include <cstdio>

namespace Nome::Scene
{

class CMeshMerger : public CMesh
{
    DEFINE_INPUT(float, Level) { MarkDirty(); }
    DEFINE_INPUT(float, Height) { MarkDirty(); }
    DEFINE_INPUT(float, Width) { MarkDirty(); }

    void UpdateEntity() override;
    // void MarkDirty() override;


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


    bool offset(DSMesh& _m);


    // sd_flag can be set to sharp and plain cc to have different types of subdivision
    void Catmull();

    bool subdivide(DSMesh& _m, unsigned int n) const;


    void setSubLevel(int level) {
        subdivisionLevel = level;
    }


    void SetSharp(bool setSharp) { isSharp = setSharp; }

    void SetOffsetFlag(bool flag) { offsetFlag = flag; }



private:
    DSMesh MergedMesh;
    std::pair<Vertex*, float> FindClosestVertex(const tc::Vector3& pos);

    unsigned int VertCount = 0;
    unsigned int FaceCount = 0;


    OpenMesh::VPropHandleT<CMeshImpl::Point> vp_pos_; // next vertex pos
    OpenMesh::EPropHandleT<CMeshImpl::Point> ep_pos_; // new edge pts
    OpenMesh::FPropHandleT<CMeshImpl::Point> fp_pos_; // new face pts

    //CMeshImpl MergedMesh;

    unsigned int subdivisionLevel = 0;
    bool isSharp = true;
    // true == NOME_OFFSET_DEFAULT, false == NOME_OFFSET_GRID
    bool offsetFlag = true;
};

}

