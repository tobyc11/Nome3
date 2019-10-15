//
// Created by peter on 10/14/19.
//

#pragma once
#include <string>

namespace Nome::PartialEdgeDS {

enum class EType {
    FACE,
    EDGE,
    VERTEX,
    PFACE,
    PEDGE,
    PVERTEX
};


class Entity
{
public:
    Entity(std::string name) : Name(std::move(name)) { }
private:
    std::string Name;
};

}
