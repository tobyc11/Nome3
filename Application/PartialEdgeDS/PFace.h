//
// Created by peter on 10/14/19.
//

#pragma once
#include "Entity.h"
#include "Shell.h"
#include "PFace.h"

namespace Nome::PartialEdgeDS {

class PFace: public Entity
{
public:
    Shell *shell;
    PFace *next;
    Entity *child; // Can be a face, edge, or vertex
    PFaceChildType type;
    // Extra Information
    //TODO: Add Orientation Flag
    PFace *mate;
};

}