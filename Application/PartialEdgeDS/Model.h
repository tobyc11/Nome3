//
// Created by peter on 10/14/19.
//

#pragma once
#include "Entity.h"

namespace Nome::PartialEdgeDS {

class Region;

class Model: public Entity
{
public:
    Model *next;
    Model *prev;
    Region *region;

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

    Model* killModel();
};

}