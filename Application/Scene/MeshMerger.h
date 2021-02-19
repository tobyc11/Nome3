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

    bool subdivide(DSMesh& _m, unsigned int n);



    void SetSharp(bool setSharp) {isSharp = setSharp;}


    void setSubLevel(int level) {
        subdivisionLevel = level;
    }


    // hacky way to fix Mobius face, where CCW goes to CW edge orientation
    std::vector<Vector3> fixEdgeOrientation(std::vector<Vector3> facePositions);

    
    
    //below are new

    // Constructor.
    //Subdivision();

    // Constructor.
    // @param mesh: the reference of given mesh.
    //Subdivision(Mesh mesh);

    // The integration of subdivision.
    // @param level: level of Catmull-Clark subdivision.


private:
    DSMesh MergedMesh;
    std::pair<Vertex*, float> FindClosestVertex(const tc::Vector3& pos);

    unsigned int VertCount = 0;
    unsigned int FaceCount = 0;


    unsigned int subdivisionLevel = 0;
    bool isSharp = true;


};


}




