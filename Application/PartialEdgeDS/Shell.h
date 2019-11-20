//
// Created by peter on 10/14/19.
//

#pragma once
#include "Entity.h"

namespace Nome::PartialEdgeDS {

class Region;
class PFace;

class Shell: public Entity
{
public:
    Region *region;
    Shell *next;
    Shell *prev;
    PFace *pFace;

    Shell(const Region *&region, const Shell *&next, const Shell *&prev, const pFace *&pFace);
    ~Shell();
};

}