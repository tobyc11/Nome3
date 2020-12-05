#include <opensubdiv/far/topologyRefinerFactory.h>
#include <opensubdiv/far/primvarRefiner.h>
#include <opensubdiv/far/topologyDescriptor.h>
#include <array>

///temp
#include <OpenMesh/Core/IO/MeshIO.hh>



using namespace OpenSubdiv;

Sdc::SchemeType SubdivisionType() {
    return Sdc::SCHEME_CATMARK;
}
Sdc::Options SubdivisionOptions() {
    Sdc::Options options;

    options.SetVtxBoundaryInterpolation(Sdc::Options::VTX_BOUNDARY_EDGE_ONLY);
    // options.SetVtxBoundaryInterpolation(Sdc::Options::VTX_BOUNDARY_NONE);
    // options.SetVtxBoundaryInterpolation(Sdc::Options::VTX_BOUNDARY_NONE);
    // options.SetCreasingMethod(Sdc::Options::CREASE_CHAIKIN);

    return options;
}


Far::TopologyRefiner * GetRefiner(CMeshImpl& _m, bool isSharp) {
    typedef Far::TopologyDescriptor Descriptor;

    Descriptor desc;
    // convert the mesh information for subdivision

    /// assign topology
    {
        desc.numVertices = _m.n_vertices();
        desc.numFaces = _m.n_faces();
        int *faceVerts = new int[_m.n_faces()];
        int *faceVertsIndices = new int[_m.n_faces() * 4];
        for (auto face : _m.faces()) {
            faceVerts[face.idx()] = 4;
            int i = 0;
            for (auto vert : face.vertices()) {
                faceVertsIndices[4 * face.idx() + i] = vert.idx();
                i++;
            }

        }
        desc.numVertsPerFace = faceVerts;


        desc.vertIndicesPerFace = faceVertsIndices;
    }

    if (isSharp)
    {

        /// assign vertex sharpness
        {
            desc.numCorners = _m.n_vertices();
            // assign sharpness
            auto *sharpness = new float[_m.n_vertices()];
            // assign edge pair
            int *vertex = new int[_m.n_vertices()];
            for (auto v_itr : _m.vertices())
            {
                sharpness[v_itr.idx()] = _m.data(v_itr).sharpness();
                vertex[v_itr.idx()] = v_itr.idx();
            }
            desc.cornerWeights = sharpness;
            desc.cornerVertexIndices = vertex;
        }

        /// assign crease for edge pairs
        {
            desc.numCreases = _m.n_edges();
            // assign crease
            auto *creases = new float[_m.n_edges()];
            // assign edge pair
            int *edgePair = new int[_m.n_edges() * 2];

            for (auto edge : _m.edges())
            {
                creases[edge.idx()] = _m.data(edge).sharpness();
                edgePair[edge.idx() * 2] = edge.v0().idx();
                edgePair[edge.idx() * 2 + 1] = edge.v1().idx();
            }
            desc.creaseVertexIndexPairs = edgePair;
            desc.creaseWeights = creases;
        }
    }

    return Far::TopologyRefinerFactory<Descriptor>::Create(desc,
                                                    Far::TopologyRefinerFactory<Descriptor>::Options(SubdivisionType(), SubdivisionOptions()));

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




// commented for future switch
/*
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
<<<<<<< HEAD
    for (auto face : conv.faces()) {
        int i = 0;
        for (auto vertex : face.vertices())
            i++;
        setNumBaseFaceVertices(refiner, face.idx(), i);
=======
    for (int face=0; face<nfaces; ++face) {

        setNumBaseFaceVertices(refiner, face, 4);
>>>>>>> subdivision
    }


    // Edges and edge-faces
    int nedges = conv.n_edges();
    setNumBaseEdges(refiner, nedges);
<<<<<<< HEAD
    for (auto edge : conv.edges()) {
        setNumBaseEdgeFaces(refiner, edge.idx(), 2);
    }
=======
>>>>>>> subdivision

    // Vertices and vert-faces and vert-edges
    setNumBaseVertices(refiner, conv.n_vertices());
<<<<<<< HEAD
    for (auto vertex : conv.vertices())
=======
    printf("=================== list obj vertices===================\n ");

    for (auto v_itr = conv.vertices_begin(); v_itr != conv.vertices_end() ; ++v_itr)
>>>>>>> subdivision
    {
        printf("v %f %f %f\n", conv.point(v_itr)[0], conv.point(v_itr)[1], conv.point(v_itr)[2]);
        int i = 0;
        for (auto edge : vertex.edges())
            i++;
        setNumBaseVertexEdges(refiner, vertex.idx(), i);
        i = 0;
        for (auto face : vertex.faces())
            i++;
        setNumBaseVertexFaces(refiner, vertex.idx(), i);
    }

    return true;
}


template <>
bool
TopologyRefinerFactory<CMeshImpl>::assignComponentTopology(
    TopologyRefiner & refiner, CMeshImpl const & conv) {

    { // Face relations:
<<<<<<< HEAD
        for (auto face : conv.faces())
=======
        printf("=================== list obj face===================\n ");
        for (auto f_itr = conv.faces_begin(); f_itr != conv.faces_end() ; ++f_itr)
>>>>>>> subdivision
        {
            Far::IndexArray dstFaceVerts = getBaseFaceVertices(refiner, face.idx());
            Far::IndexArray dstFaceEdges = getBaseFaceEdges(refiner, face.idx());
            int i = 0;
<<<<<<< HEAD
            for (auto vert : face.vertices()) {
                dstFaceVerts[i] = vert.idx();
=======

            printf("f ");
            for (auto vert : f_itr->vertices()) {

                dstFaceVerts[i] = vert.idx() + 1;
                printf("%d ", vert.idx() + 1);

>>>>>>> subdivision
                i++;
            }
            printf("\n");
            i = 0;
<<<<<<< HEAD
            for (auto edge : face.edges())
=======

            for (auto edge : f_itr->edges())
>>>>>>> subdivision
            {
                dstFaceEdges[i] = edge.idx() + 1;
                i++;
            }

        }

    }

    {

        for (auto edge : conv.edges()) {
            //  Edge-vertices:
            Far::IndexArray dstEdgeVerts = getBaseEdgeVertices(refiner, edge.idx());
            dstEdgeVerts[0] = edge.v1().idx();
            dstEdgeVerts[1] = edge.v0().idx();

            //  Edge-faces
            Far::IndexArray dstEdgeFaces = getBaseEdgeFaces(refiner, edge.idx());
            dstEdgeFaces[0] = edge.h1().face().idx();
            dstEdgeFaces[1] = edge.h0().face().idx();

        }
    }


    { // Vertex relations

        for (auto v_itr = conv.vertices_begin(); v_itr != conv.vertices_end() ; ++v_itr)
        {
            Far::IndexArray vertFaces = getBaseVertexFaces(refiner, v_itr->idx());

            int i = 0;
            for (auto face : v_itr->faces()) {
                vertFaces[i] = face.idx() + 1;
                i++;
            }
            //  Vert-Edges:
<<<<<<< HEAD
            Far::IndexArray vertEdges = getBaseVertexEdges(refiner, v_itr->idx());
=======
            IndexArray vertEdges = getBaseVertexEdges(refiner, v_itr->idx());
            //LocalIndexArray vertInEdgeIndices = getBaseVertexEdgeLocalIndices(refiner, vert);

>>>>>>> subdivision
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
<<<<<<< HEAD
        ///setBaseVertexSharpness(refiner, vertex, conv.data(conv.vertex_handle(vertex)).sharpness());
=======
        setBaseVertexSharpness(refiner, vertex, 0);
>>>>>>> subdivision

    }
    return true;
}
} // namespace Far

} // namespace OPENSUBDIV_VERSION
} // namespace OpenSubdiv
<<<<<<< HEAD
 */

