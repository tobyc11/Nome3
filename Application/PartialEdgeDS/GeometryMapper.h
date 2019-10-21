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

    //OBJECT METHODSs
    //getPoint(id)
    //getPoints(ids)
    //getCurve(id)
    //getCurves(ids)
    //getSurface(id)
    //getSurfaces(id)

private:
    //OBJECT VARIABLES
    //Hash Table SURFACES:  id -> surface geometric entity
    //Hash Table CURVES:    id -> curve geometric entity
    //Hash Table POINT:     id -> point geometric entity
};

}