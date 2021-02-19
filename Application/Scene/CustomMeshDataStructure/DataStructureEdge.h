/**
 * @author Andy Wang, UC Berkeley.
 * Copyright 2016 reserve.
 * UC Berkeley, NOME project.
 * Advised by Prof. Sequin H. Carlos.
 */

#ifndef __EDGE_H__
#define __EDGE_H__

#include "DataStructureVertex.h"

#include <iostream>


using namespace std;

class Vertex;
class Face;

//////////////////////////////////////////////////////////////////////
// Edge Class -- Edge connects Vertex A and Vertex B.
//
class Edge
{
public:
    int edge_ID = 0;
    // Pointer to Vertex A of this edge.
    Vertex* va;
    // Pointer to Vertex B of this edge.
    Vertex* vb;
    // Pointer to Face A of this edge.
    Face* fa;
    // Pointer to Face B of this edge.
    Face* fb;
    // Next edge on the side of Va and Fa.
    Edge* nextVaFa;
    // Next edge on the side of Va and Fb.
    Edge* nextVaFb;
    // Next edge on the side of Vb and Fa.
    Edge* nextVbFa;
    // Next edge on the side of Vb and Fb.
    Edge* nextVbFb;
    // A pointer to the edge point after makeEdgePoints in subdivision.
    Vertex* edgePoint;
    // A pointer to the first half edge in subdivision.
    Edge* firstHalf;
    // A pointer to the second half edge in subdivision.
    Edge* secondHalf;
    // A flag of mobius. If two faces are with same orientation, false. Otherwise, true.
    bool mobius;
    // Constructor.
    Edge();
    // Constructor with start and end vertex.
    // @param va, vb: pointers to the va and vb of this edge.
    Edge(Vertex* va, Vertex* vb);
    // Find the corresponding next edge given a vertex and face of this edge.
    // Return an error if the vertex or face is not adjacent to this edge.
    Edge* nextEdge(Vertex* v, Face* f);
    // Set the corresponding next edge with a vertex and face of this edge.
    // Return an error if the vertex or face is not adjacent to this edge.
    void setNextEdge(Vertex* v, Face* f, Edge* nextEdge);
    // Find the other point of this edge
    // @param v, the known vertex
    Vertex* theOtherVertex(Vertex* v);
    // Find the other face of this edge
    // @param f, the known face
    Face* theOtherFace(Face* f) const;
    // The next edge in a face traversal of f.
    Edge* nextEdgeOfFace(Face* f);
    int idx() const;
    float sharpness;
    Vertex* v0() const;
    Vertex* v1() const;
};

#endif // __EDGE_H__
