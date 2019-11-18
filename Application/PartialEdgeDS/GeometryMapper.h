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
#include <string>
#include "Geometry.h"

namespace Nome::PartialEdgeDS

{

enum EGType{
    POINT,
    CURVE,
    SURFACE
};

class GeometryMapper
{
public:
    // Constructors & destructors
    GeometryMapper();
    ~GeometryMapper();

    // Public methods
    bool killModel(u_int64_t modelUID);
    //After killing, the map at modelUID no longer exits
    bool killGeometry(u_int64_t modelUID, u_int64_t geometryUID);

    bool addGeometry(u_int64_t modelUID,
                     EGType type,
                     u_int64_t geometryUID,
                     Geometry *geometry);

    bool copyModel(const u_int64_t &modelUID, u_int64_t &newModelUID);
    
    //After merging, the map at bModelUID no longer exits
    std::map<std::pair<EGType, u_int64_t>, u_int64_t> *mergeModels(u_int64_t aModelUID,
                                                                   u_int64_t bModelUID);

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
    std::map<u_int64_t, std::map<u_int64_t, std::pair<EGType, Entity *>>> map;
    UIDGenerator idGenerator;

    // Private methods
    std::pair<EGType, u_int64_t> geometry_key(EGType type, u_int64_t geometryUID);
};
}

