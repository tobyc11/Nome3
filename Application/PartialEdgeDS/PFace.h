//
// Created by peter on 10/14/19.
//

#pragma once
#include "Entity.h"

namespace Nome::PartialEdgeDS {

class Shell;
class PFace;

class PFace: public Entity
{
public:
    Shell *shell;
    PFace *next;
    PFace *prev;
    Entity *child; // Can be a face, edge, or vertex
    EType type; // TODO: Refactor this type to be child_type
    // Extra Information
    //TODO: Add Orientation Flag
    PFace *mate;

    PFace();
    PFace(const Shell *&shell, const PFace *&next, const PFace *&prev, const Entity &*child, const EType &type);
    ~PFace();
};

}