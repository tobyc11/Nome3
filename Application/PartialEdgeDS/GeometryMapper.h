//
// Created by Peter Generao on 10/21/19.
//

/* Will act as an interface between the PartialEdgeDS and actual geometric
 * entities. This way, we decouple geometry objects from the data structure
 * which references them. Will make duplicating geometry (and entire models)
 * easier to implement. */

namespace Nome::PartialEdgeDS

{

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
     *  * AddGeom(model_uid, geom_uid)
     *      - for POINTS
     *      - for CURVES
     *      - for SURFACE
     *  * CopyModel(model_uid) -> (deep copy and move to new index)
     * 
     *  * getEntity(model_id, G_TYPE, geom_id) 
     *  * getAllPoints(model_id)
     *  * getAllCurves(model_id)
     *  * getAllSurfaces(model_id)
     *  * model_merge(model_a_id, model_b_id)
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

private:
    //OBJECT VARIABLES
    //Hash Table MODELS
    //


    //TODO: once we implement GeometryMapper, we can get rid of flags, and just use HoFs
};

}