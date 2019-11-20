//
// Created by peter on 10/14/19.
//

#pragma once
#include "Entity.h"
#include <vector>

namespace Nome::PartialEdgeDS {

class Model;
class Shell;
class Face;

class Region: public Entity
{
public:
    Model *model;
    Region *next;
    Region *prev;
    Shell *shell;

    Region(const Model *&model, const Region *&next, const Region *&prev, const Shell *&shell);
    ~Region();

    std::vector<Face *> getFaces() const;
};

}