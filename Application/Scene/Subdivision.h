#include <opensubdiv/far/topologyRefinerFactory.h>
#include <opensubdiv/far/primvarRefiner.h>

///temp
#include <OpenMesh/Core/IO/MeshIO.hh>



using namespace OpenSubdiv;

Sdc::SchemeType subdivisionType() {
    return Sdc::SCHEME_CATMARK;
}
Sdc::Options subdivisionOptions() {
    Sdc::Options options;
    options.SetVtxBoundaryInterpolation(Sdc::Options::VTX_BOUNDARY_NONE);
    return options;
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
        //TODO add specific number
        setNumBaseFaceVertices(refiner, face, 4);
    }

    // Edges and edge-faces
    int nedges = conv.n_edges();
    setNumBaseEdges(refiner, nedges);

    // Vertices and vert-faces and vert-edges

    setNumBaseVertices(refiner, conv.n_vertices());
    printf("=================== list obj vertices===================\n ");

    for (auto v_itr = conv.vertices_begin(); v_itr != conv.vertices_end() ; ++v_itr)
    {
        printf("v %f %f %f\n", conv.point(v_itr)[0], conv.point(v_itr)[1], conv.point(v_itr)[2]);
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
        printf("=================== list obj face===================\n ");
        for (auto f_itr = conv.faces_begin(); f_itr != conv.faces_end() ; ++f_itr)
        {
            IndexArray dstFaceVerts = getBaseFaceVertices(refiner, f_itr->idx());
            IndexArray dstFaceEdges = getBaseFaceEdges(refiner, f_itr->idx());
            int i = 0;

            printf("f ");
            for (auto vert : f_itr->vertices()) {

                dstFaceVerts[i] = vert.idx() + 1;
                printf("%d ", vert.idx() + 1);

                i++;
            }
            printf("\n");
            i = 0;

            for (auto edge : f_itr->edges())
            {
                dstFaceEdges[i] = edge.idx() + 1;
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
                vertFaces[i] = face.idx() + 1;
                i++;
            }
            //  Vert-Edges:
            IndexArray vertEdges = getBaseVertexEdges(refiner, v_itr->idx());
            //LocalIndexArray vertInEdgeIndices = getBaseVertexEdgeLocalIndices(refiner, vert);

            i = 0;
            for (auto edge : v_itr->edges()) {
                vertEdges[i] = edge.idx() + 1;
            }
        }

    }

    populateBaseLocalIndices(refiner);
    if ( ! OpenMesh::IO::write_mesh(conv, "/Users/charleszhang/code/nome/nome/temp.obj") )
    {
        std::cerr << "Error: cannot write mesh to " << "/Users/charleszhang/code/nome/nome/temp.obj" << std::endl;
        return 1;
    }

    return true;
};



template <>
bool
TopologyRefinerFactory<CMeshImpl>::assignComponentTags(
    TopologyRefiner & refiner, CMeshImpl const & conv) {

    // arbitrarily sharpen the 4 bottom edges of the pyramid to 2.5f
    for (int vertex=0; vertex < conv.n_vertices(); ++vertex) {
        setBaseVertexSharpness(refiner, vertex, 0);

    }
    return true;
}
} // namespace Far

} // namespace OPENSUBDIV_VERSION
} // namespace OpenSubdiv

