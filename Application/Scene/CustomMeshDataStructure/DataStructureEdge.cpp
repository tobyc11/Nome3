/**
 * @author Andy Wang, UC Berkeley.
 * Copyright 2016 reserve.
 * UC Berkeley, Slide_2016 project.
 * Advised by Prof. Sequin H. Carlos.
 */

#include "DataStructureEdge.h"

Edge::Edge()
{
    va = vb = NULL;
    nextVaFa = nextVaFb = nextVbFa = nextVbFb = NULL;
    fa = fb = NULL;
    edgePoint = NULL;
    mobius = false;
    firstHalf = secondHalf = NULL;
    sharpness = 0;
}

Edge::Edge(Vertex* v1, Vertex* v2)
{
    va = v1;
    vb = v2;
    nextVaFa = nextVaFb = nextVbFa = nextVbFb = NULL;
    fa = fb = NULL;
    edgePoint = NULL;
    mobius = false;
    firstHalf = secondHalf = NULL;
    sharpness = 0;
}

Edge* Edge::nextEdge(Vertex* v, Face* f)
{
    if (v == va)
    {
        if (f == fa)
        {
            return nextVaFa;
        }
        else if (f == fb)
        {
            return nextVaFb;
        }
    }
    else if (v == vb)
    {
        if (f == fa)
        {
            return nextVbFa;
        }
        else if (f == fb)
        {
            return nextVbFb;
        }
    }
    std::cout << "Error: Invalid search of edge at vertex " << v->ID << "." << std::endl;
    exit(0);
}

void Edge::setNextEdge(Vertex* v, Face* f, Edge* nextEdge)
{
    if (v == va)
    {
        if (f == fa)
        {
            nextVaFa = nextEdge;
            return;
        }
        else if (f == fb)
        {
            nextVaFb = nextEdge;
            return;
        }
    }
    else if (v == vb)
    {
        if (f == fa)
        {
            nextVbFa = nextEdge;
            return;
        }
        else if (f == fb)
        {
            nextVbFb = nextEdge;
            return;
        }
    }
    std::cout << "Error: Invalid set next edge at vertex " << v->ID << "." << std::endl;
    exit(0);
}

Vertex* Edge::theOtherVertex(Vertex* v)
{
    if (v == va)
    {
        return vb;
    }
    else if (v == vb)
    {
        return va;
    }
    else
    {
        std::cout << "ERROR: v is not in edge!" << std::endl;
        // exit(0);
    }
}

Face* Edge::theOtherFace(Face* f) const
{
    if (f == fa)
    {
        return fb;
    }
    else if (f == fb)
    {
        return fa;
    }
    else
    {
        std::cout << "ERROR: f is not adjacent to this edge" << std::endl;
        exit(0);
    }
}

Edge* Edge::nextEdgeOfFace(Face* f)
{
    if (f == fa)
    {
        return nextVbFa;
    }
    else if (f == fb)
    {
        if (mobius)
        {
            return nextVbFb;
        }
        else
        {
            return nextVaFb;
        }
    }
    else
    {
        std::cout << "ERROR: f is not adjacent to this edge" << std::endl;
        exit(0);
    }
}
int Edge::idx() const {
    return edge_ID;
}
Vertex* Edge::v0() const {
    return va;
}
Vertex* Edge::v1() const {
    return vb;
}
