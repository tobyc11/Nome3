//
// Created by peter on 10/13/19.
//
#pragma once
#include <memory>
#include <string>
#include <vector>
#include <typeinfo>

namespace Nome::Scene::PartialEdgeDS
{

/**
 * The Partial Edge Data Structure that we will be implementing will closely
 * correspond to the paper "Partial Entity Structure: A Compact Non-Manifold
 * Boundary Representation Based on Partial Topological Entities", by Sang Hun
 * Lee and Kunwoo Lee; however, we will be enforcing a strict linked list
 * structure."
 *
*/

class Entity
{
public:
    Entity(std::string name) : Name(std::move(name)) { }
private:
    std::string Name;
};

class Region;
class Shell;

class Model: public Entity
{
public:
    Model *next;
    Region *region;
};

class Region: public Entity
{
public:
    Model *model;
    Region *next;
    Shell *shell;

    std::vector<PFace> getFaces() const;
};

class Shell: public Entity
{
public:
    Region *region;
    Shell *next;
    PFace *pface;
};

enum PFaceChildType {
    face,
    edge,
    vertex
};

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

class Face: public Entity
{
public:
    PFace *pface;
    Loop *loop;
    //TODO: Add Surface Geometry
};



class Loop: public Entity
{
public:
    Face *face;
    Loop *next;
    PEdge *pedge;
};

class PEdge: public Entity
{
public:
    Loop *loop;
    Entity *child; // Can be a edge, or vertex

    // Extra Information
    //TODO: Add Orientation Flag
    PVertex *pvertex; // Start pvertex
    PEdge *looped_prev;
    PEdge *looped_next;
    PEdge *radial_prev;
    PEdge *radial_next;
};

class Edge: public Entity
{
public:
    Entity *parent; // Can be a partial face, or partial edge
    PVertex *pvertices[2];
    //TODO: Add Curve Geometry
};

class PVertex: public Entity
{
public:
    Edge *edge;
    PVertex *next; //TODO: Evaluate if we actually need this
    Vertex *vertex;
};

class Vertex: public Entity
{
public:
    Entity *parent; // Can be a partial face, partial edge,
                                    // or partial vertex
    //TODO: Add Point Geometry
};

std::vector<Face *> Region::getFaces() const
{
    std::vector<Face *> pFaces;
    Shell *tempShellP = shell;

    while (tempShellP != null)
    {
        PFace *tempPFaceP = tempShellP->pface;
        while (tempPFaceP != null)
        {
            if (tempPFaceP->type == face) { pFaces.push_back((Face *)tempPFaceP->child); }
            tempPFaceP = tempPFaceP->next;
        }
        tempShellP = tempShellP->next;
    }
    return pFaces;
}

} /* namespace Nome::Scene::PartialEdgeDS */