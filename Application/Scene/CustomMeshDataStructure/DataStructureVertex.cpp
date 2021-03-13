/**
 * @author Andy Wang, UC Berkeley.
 * Copyright 2016 reserve.
 * UC Berkeley, Slide_2016 project.
 * Advised by Prof. Sequin H. Carlos.
 */


#include "DataStructureVertex.h"
#include <iostream>

Vertex::Vertex()
{
    position = tc::Vector3(0, 0, 0);
    normal = tc::Vector3(0, 0, 0);
    oneEdge = NULL;
    this->ID = 0;
    selected = false;
    isParametric = false;
    before_transform_vertex = NULL;
    source_vertex = NULL;
}

Vertex::Vertex(float x, float y, float z, unsigned long ID)
{
    position = tc::Vector3(x, y, z);
    normal = tc::Vector3(0, 0, 0);
    oneEdge = NULL;
    this->ID = ID;
    selected = false;
    isParametric = false;
    before_transform_vertex = NULL;
}

Vertex::Vertex(float x, float y, float z, std::string assignedName, unsigned long ID)
{
    name = assignedName;
    position = tc::Vector3(x, y, z);
    normal = tc::Vector3(0, 0, 0);
    oneEdge = NULL;
    this->ID = ID; // avoid naming confusion
    selected = false;
    isParametric = false;
    before_transform_vertex = NULL;
}

void Vertex::update()
{
   /* if (isParametric)
    {
        float new_x, new_y, new_z;
        if (x_expr != "")
        {
            new_x = evaluate_expression(x_expr, params);
        }
        else
        {
            new_x = position[0];
        }
        if (y_expr != "")
        {
            new_y = evaluate_expression(y_expr, params);
        }
        else
        {
            new_y = position[1];
        }
        if (z_expr != "")
        {
            new_z = evaluate_expression(z_expr, params);
        }
        else
        {
            new_z = position[2];
        }
        position = vec3(new_x, new_y, new_z);
    }*/
}
