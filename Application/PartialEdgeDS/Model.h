//
// Created by peter on 10/14/19.
//

#pragma once
#include "Entity.h"
#include <string>
#include <vector>

namespace Nome::PartialEdgeDS {

class Region;

class Model: public Entity
{
public:
    Model *next;
    Model *prev;
    Region *region;
    u_int64_t modelUID;

    Model();
    ~Model();

    /**
     * TODO: Implement Merge Models for merging A and B
     *  - First, Merge HashTables, get list of new B ids
     *  - Apply changes to B for ids at Faces, Edges, Vertices
     *  - Append Region(s) from B to A
     *  - Null out *region
     *  - Kill B -> return pointer to next guy after B
     *  - Stitch to previous
     *  - Free B
    */

    /**
   *  TODO: OBJECT MEMBERS/METHODS FOR GEOMETRY MAPPER
   *  * Free Model
   *  * AddPoint(to edge_uid)
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

  */
    bool cutFaceByTwoVertexes(const std::string &vertexID1, const std::string &vetexID2);
    std::vector<Region *> getRegions() const;
    Model* killModel();
    void mergeModel(Model *bModel);
    void changeKeysInModel(std::map<std::pair<EGType, u_int64_t>, u_int64_t> *keyMap);
};

}
