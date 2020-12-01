#pragma once
#include "Mesh.h"
#include <LangUtils.h>
#include <opensubdiv/far/topologyRefinerFactory.h>
#include <opensubdiv/far/primvarRefiner.h>

#include <cstdio>

using namespace OpenSubdiv;

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
        : CMesh(std::move(name))
    {
    }


    // No update yet, please just use one time
    void MergeIn(CMeshInstance& meshInstance, bool markedSharp = false);

    void MergeClear();

    // Buggy, preset to 3 subdivision steps
    void Catmull();

    bool subdivide(CMeshImpl& _m, unsigned int n, bool _update_points);

    void split_face(CMeshImpl& _m, const CMeshImpl::FaceHandle& _fh);

    void split_edge(CMeshImpl& _m, const CMeshImpl::EdgeHandle& _eh);

    void compute_midpoint(CMeshImpl& _m, const CMeshImpl::EdgeHandle& _eh, bool _update_points);

    void update_vertex( CMeshImpl& _m, const CMeshImpl::VertexHandle& _vh);

    bool prepare(CMeshImpl& _m);

    bool cleanup(CMeshImpl& _m);

    void setSubLevel(int level) {
        subdivisionLevel = level;
    }

private:
    Sdc::SchemeType subdivisionType() const {
        return Sdc::SCHEME_CATMARK;
    }
    Sdc::Options subdivisionOptions() const {
        Sdc::Options options;
        options.SetVtxBoundaryInterpolation(Sdc::Options::VTX_BOUNDARY_EDGE_ONLY);
        return options;
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
    friend struct Converter;


};

}


/*
struct Converter {

public:
    friend struct Converter;

    Sdc::SchemeType GetType() const {
        return Sdc::SCHEME_CATMARK;
    }

    Sdc::Options GetOptions() const {
        Sdc::Options options;
        options.SetVtxBoundaryInterpolation(Sdc::Options::VTX_BOUNDARY_EDGE_ONLY);
        return options;
    }

    int GetNumFaces() const { return Merged; }

    int GetNumEdges() const { return g_nedges; }

    int GetNumVertices() const { return g_nverts; }

    //
    // Face relationships
    //
    int GetNumFaceVerts(int face) const { return g_facenverts[face]; }

    int const * GetFaceVerts(int face) const { return g_faceverts+getCompOffset(g_facenverts, face); }

    int const * GetFaceEdges(int face) const { return g_faceedges+getCompOffset(g_facenverts, face); }


    //
    // Edge relationships
    //
    int const * GetEdgeVertices(int edge) const { return g_edgeverts+edge*2; }

    int GetNumEdgeFaces(int edge) const { return g_edgenfaces[edge]; }

    int const * GetEdgeFaces(int edge) const { return g_edgefaces+getCompOffset(g_edgenfaces, edge); }

    //
    // Vertex relationships
    //
    int GetNumVertexEdges(int vert) const { return g_vertexnedges[vert]; }

    int const * GetVertexEdges(int vert) const { return g_vertexedges+getCompOffset(g_vertexnedges, vert); }

    int GetNumVertexFaces(int vert) const { return g_vertexnfaces[vert]; }

    int const * GetVertexFaces(int vert) const { return g_vertexfaces+getCompOffset(g_vertexnfaces, vert); }

private:

    int getCompOffset(int const * comps, int comp) const {
        int ofs=0;
        for (int i=0; i<comp; ++i) {
            ofs += comps[i];
        }
        return ofs;
    }

};
 */


