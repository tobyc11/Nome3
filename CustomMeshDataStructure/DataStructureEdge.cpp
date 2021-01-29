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
    isSharp = false;
    mobius = false;
    firstHalf = secondHalf = NULL;
}

Edge::Edge(Vertex* v1, Vertex* v2)
{
    va = v1;
    vb = v2;
    nextVaFa = nextVaFb = nextVbFa = nextVbFb = NULL;
    fa = fb = NULL;
    edgePoint = NULL;
    isSharp = false;
    mobius = false;
    firstHalf = secondHalf = NULL;
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
    cout << "Error: Invalid search of edge at vertex " << v->ID << "." << endl;
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
    cout << "Error: Invalid set next edge at vertex " << v->ID << "." << endl;
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
        cout << "ERROR: v is not in edge!" << endl;
        // exit(0);
    }
}

Face* Edge::theOtherFace(Face* f)
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
        cout << "ERROR: f is not adjacent to this edge" << endl;
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
        cout << "ERROR: f is not adjacent to this edge" << endl;
        exit(0);
    }
}
