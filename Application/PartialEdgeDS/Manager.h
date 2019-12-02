//
// Created by Peter Generao on 10/21/19.
//

/* Will act as an interface between the PartialEdgeDS and actual geometric
 * entities. This way, we decouple geometry objects from the data structure
 * which references them. Will make duplicating geometry (and entire models)
 * easier to implement. */


/**
 *  TODO: OBJECT MEMBERS/METHODS FOR GEOMETRY MAPPER
 *  * M[Model_ID][Geometry_Type][Geom_ID]
 *
 *  * KillModel(modelUID)
 *  * AddGeom(modelUID, Geometric *Object) >> returns geom_id to where it was created
 *      - for POINTS
 *      - for CURVES
 *      - for SURFACE
 *  * Private: CopyModel(modelUID) -> (deep copy and move to new index) >> id where the copy is located
 *
 *  * getEntity(model_id, G_TYPE, geom_id)
 *  * getAllPoints(model_id)
 *  * getAllCurves(model_id)
 *  * getAllSurfaces(model_id)
 *  * model_append(model_a_id, model_b_id) Model A gets modified, b gets deleted
 *      -   Merge Hash Tables, record collisions
 *      -   return Map of B_old_Id -> B_new_ID (all of them, not just collisions)
 *
 *  * applyTransformation(model_id, T)
 *  *  T ~ lambda x,y,z -> x', y', z
*/


#pragma once
#include <map>
#include <vector>
#include <string>
#include "Geometry.h"

namespace Nome::PartialEdgeDS

{

enum EGType{
    POINT,
    CURVE,
    SURFACE
};

class Manager
{
public:
    // Constructors & destructors
    Manager();
    ~Manager();

    // Public methods
    bool addModel(u_int64_t &modelUID);
    bool killModel(const u_int64_t &modelUID);
    //After killing, the map at modelUID no longer exits
    bool killGeometry(const u_int64_t &modelUID, const u_int64_t &geometryUID);

    bool addGeometry(const u_int64_t &modelUID,
                     const EGType &type,
                     const u_int64_t &geometryUID,
                     const Geometry *&geometry);

    bool getEdgesOfVertex(const u_int64_t &modelUID, const u_int64_t &vertexUID, std::vector<Geometry *> &edges);
    bool getEdgesOfFace(const u_int64_t &modelUID, const u_int64_t &faceUID, std::vector<Geometry *> &edges);
    bool getVertexesOfEdge(const u_int64_t &modelUID, const u_int64_t &edgeUID, std::vector<Geometry *> &vertexes);
    bool getVertexesOfFace(const u_int64_t &modelUID, const u_int64_t &faceUID, std::vector<Geometry *> &vertexes);

    bool makeVertex(const u_int64_t &modelUID, const Geometry *&point, u_int64_t &vertexUID);
    bool killVertex(const u_int64_t &modelUID, const u_int64_t &vertexUID);

    bool MEV(const u_int64_t &modelUID, const u_int64_t &fromVertexUID, const Geometry *&point, u_int64_t &toVertexUID, u_int64_t &edgeUID);
    bool MFE(const u_int64_t &modelUID, const std::vector<u_int64_t> &edges, u_int64_t &faceUID);

    bool KEV(const u_int64_t &modelUID, const u_int64_t &edgeUID);
    bool KFE(const u_int64_t &modelUID, const u_int64_t &edgeUID, const u_int64_t &faceUID);

    bool copyModel(const u_int64_t &modelUID, u_int64_t &newModelUID);

    //After merging, the map at bModelUID no longer exits
    bool mergeModels(const u_int64_t &aModelUID,
                     const u_int64_t &bModelUID,
                     std::map<std::pair<EGType, u_int64_t>, u_int64_t> *&keysToChangeInB);

private:
    // Use for internal UID generation
    class UIDGenerator
    {
    public:
        u_int64_t newUID();

    private:
        u_int64_t counter;
    };

    // Private fields
    std::map<u_int64_t, std::map<u_int64_t, std::pair<EGType, Geometry*>>> map;
    UIDGenerator idGenerator;

    // Private methods
    std::pair<EGType, u_int64_t> geometry_key(EGType type, u_int64_t geometryUID);
};
}


