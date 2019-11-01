//
// Created by Peter Generao on 10/21/19.
//

/* Will act as an interface between the PartialEdgeDS and actual geometric
 * entities. This way, we decouple geometry objects from the data structure
 * which references them. Will make duplicating geometry (and entire models)
 * easier to implement. */

#pragma once
#include <map>
#include <string>
#include "Geometry.h"

namespace Nome::PartialEdgeDS

{

enum class EGType {
    POINT,
    CURVE,
    SURFACE
};

class GeometryMapper
{
public:
    GeometryMapper();
    ~GeometryMapper();

    /**
     *  TODO: OBJECT MEMBERS/METHODS FOR GEOMETRY MAPPER
     *  * M[Model_ID][Geometry_Type][Geom_ID]
     *
     *  * KillModel(model_uid)
     *  * AddGeom(model_uid, Geometric *Object) >> returns geom_id to where it was created
     *      - for POINTS
     *      - for CURVES
     *      - for SURFACE
     *  * Private: CopyModel(model_uid) -> (deep copy and move to new index) >> id where the copy is located
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

    //OBJECT METHODS
    //
    //
    //
    //
    bool killModel(const std::string &model_uid);

    bool addGeometry(const std::string &model_uid,
                     const EGType &type,
                     const std::string &geometry_uid,
                     const Geometry &geometry);

    bool copyModel(const std::string &model_uid);

private:
    //OBJECT VARIABLES
    //Hash Table MODELS
    //
    std::map<std::string, std::map<EGType,std::map<std::string, *Geometry>>> map;


    //TODO: once we implement GeometryMapper, we can get rid of flags, and just use HoFs
};

}