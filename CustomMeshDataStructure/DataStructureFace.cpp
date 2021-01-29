/**
 * @author Andy Wang, UC Berkeley.
 * Copyright 2016 reserve.
 * UC Berkeley, Slide_2016 project.
 * Advised by Prof. Sequin H. Carlos.
 */

#include "DataStructureFace.h"

Face::Face()
{
    oneEdge = NULL;
    facePoint = NULL;
    selected = false;
    id = -1;
    name = "";
    user_defined_color = false;
}

Face::Face(std::vector<Vertex> vertices)
{
    oneEdge = NULL;
    facePoint = NULL;
    selected = false;
    id = -1;
    name = "";
    user_defined_color = false;
    vertices = vertices;
}

