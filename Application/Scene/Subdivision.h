#include <opensubdiv/far/topologyRefinerFactory.h>
#include <opensubdiv/far/primvarRefiner.h>
#include <opensubdiv/far/topologyDescriptor.h>
#include <array>

///temp


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


Far::TopologyRefiner * GetRefiner(DSMesh& _m, bool isSharp) {
    typedef Far::TopologyDescriptor Descriptor;

    Descriptor desc;
    // convert the mesh information for subdivision

    /// assign topology
    {

        desc.numVertices = _m.n_vertices();
        desc.numFaces = _m.n_faces();
        int *faceVerts = new int[_m.n_faces()];
        int count = 0;
        for (auto face : _m.faces()) {
            int i = 0;
            for (auto vert : face->vertices) {
                i++;
                count++;
            }
            faceVerts[face->id] = i;
        }
        int *faceVertsIndices = new int[count];
        int i = 0;
        for (auto face : _m.faces()) {
            for (auto vert : face->vertices) {
                faceVertsIndices[i] = vert->ID;
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
            for (auto v_itr : _m.vertList)
            {
                sharpness[v_itr->ID] = v_itr->sharpness;
                vertex[v_itr->ID] = v_itr->ID;
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
                creases[edge->idx()] = edge->sharpness;
                edgePair[edge->idx() * 2] = edge->v0()->ID;
                edgePair[edge->idx() * 2 + 1] = edge->v1()->ID;
            }
            desc.creaseVertexIndexPairs = edgePair;
            desc.creaseWeights = creases;
        }
    }

    return Far::TopologyRefinerFactory<Descriptor>::Create(desc,
                                                    Far::TopologyRefinerFactory<Descriptor>::Options(SubdivisionType(), SubdivisionOptions()));

}





