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


};

}



struct Vertex {

    // Minimal required interface ----------------------
    Vertex() { }

    Vertex(Vertex const & src) {
        _position[0] = src._position[0];
        _position[1] = src._position[1];
        _position[2] = src._position[2];
    }

    void Clear( void * =0 ) {
        _position[0]=_position[1]=_position[2]=0.0f;
    }

    void AddWithWeight(Vertex const & src, float weight) {
        _position[0]+=weight*src._position[0];
        _position[1]+=weight*src._position[1];
        _position[2]+=weight*src._position[2];
    }

    // Public interface ------------------------------------
    void SetPosition(float x, float y, float z) {
        _position[0]=x;
        _position[1]=y;
        _position[2]=z;
    }

    const float * GetPosition() const {
        return _position;
    }

private:
    float _position[3];
};


namespace OpenSubdiv {
namespace OPENSUBDIV_VERSION {

namespace Far {

template <>
bool
TopologyRefinerFactory<CMeshImpl>::resizeComponentTopology(
    TopologyRefiner & refiner, CMeshImpl const & conv) {

    // Faces and face-verts
    int nfaces = conv.n_faces();
    setNumBaseFaces(refiner, nfaces);
    for (int face=0; face<nfaces; ++face) {
        setNumBaseFaceVertices(refiner, face, 4);
    }

    // Edges and edge-faces
    int nedges = conv.n_edges();
    setNumBaseEdges(refiner, nedges);


    // Vertices and vert-faces and vert-edges

    setNumBaseVertices(refiner, conv.n_vertices());
    for (auto v_itr = conv.vertices_begin(); v_itr != conv.vertices_end() ; ++v_itr)
    {
        int i = 0;
        for (auto edge : v_itr->edges())
            i++;
        setNumBaseVertexEdges(refiner, v_itr->idx(), i);
        i = 0;
        for (auto face : v_itr->faces())
            i++;
        setNumBaseVertexFaces(refiner, v_itr->idx(), i);
    }
    return true;
}

template <>
bool
TopologyRefinerFactory<CMeshImpl>::assignComponentTopology(
    TopologyRefiner & refiner, CMeshImpl const & conv) {

    typedef Far::IndexArray      IndexArray;

    { // Face relations:
        for (auto f_itr = conv.faces_begin(); f_itr != conv.faces_end() ; ++f_itr)
        {
            IndexArray dstFaceVerts = getBaseFaceVertices(refiner, f_itr->idx());
            IndexArray dstFaceEdges = getBaseFaceEdges(refiner, f_itr->idx());
            int i = 0;
            for (auto vert : f_itr->vertices()) {
                dstFaceVerts[i] = vert.idx();
                i++;
            }
            i = 0;
            for (auto edge : f_itr->edges())
            {
                dstFaceEdges[i] = edge.idx();
                i++;
            }
        }

    }


    { // Vertex relations

        for (auto v_itr = conv.vertices_begin(); v_itr != conv.vertices_end() ; ++v_itr)
        {
            IndexArray vertFaces = getBaseVertexFaces(refiner, v_itr->idx());

            int i = 0;
            for (auto face : v_itr->faces()) {
                vertFaces[i] = face.idx();
                i++;
            }
            //  Vert-Edges:
            IndexArray vertEdges = getBaseVertexEdges(refiner, v_itr->idx());
            //LocalIndexArray vertInEdgeIndices = getBaseVertexEdgeLocalIndices(refiner, vert);
            /*
            for (int edge=0; edge<convGetNumVertexEdges(vert); ++edge) {
                vertEdges[edge] = conv.GetVertexEdges(vert)[edge];
            }
             */
            i = 0;
            for (auto edge : v_itr->edges()) {
                vertEdges[i] = edge.idx();
            }
        }

    }

    populateBaseLocalIndices(refiner);

    return true;
};


template <>
bool
TopologyRefinerFactory<CMeshImpl>::assignComponentTags(
    TopologyRefiner & refiner, CMeshImpl const & conv) {

    // arbitrarily sharpen the 4 bottom edges of the pyramid to 2.5f
    for (int vertex=0; vertex < conv.n_vertices(); ++vertex) {
        setBaseVertexSharpness(refiner, vertex, conv.data(conv.vertex_handle(vertex)).sharpness()/*g_edgeCreases[edge]*/);

    }
    return true;
}
} // namespace Far

} // namespace OPENSUBDIV_VERSION
} // namespace OpenSubdiv