//
// Created by peter on 10/13/19.
//
#pragma once
#include <memory>

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
    int id;
};

class Model: public Entity
{
public:
    std::shared_ptr<Model> next;
    std::shared_ptr<Region> region;
};

class Region: public Entity
{
public:
    std::shared_ptr<Model> model;
    std::shared_ptr<Region> next;
    std::shared_ptr<Shell> shell;
};

class Shell: public Entity
{
public:
    std::shared_ptr<Region> region;
    std::shared_ptr<Shell> next;
    std::shared_ptr<PFace> pface;
};

class PFace: public Entity
{
public:
    std::shared_ptr<Shell> shell;
    std::shared_ptr<PFace> next;
    std::shared_ptr<Entity> child; // Can be a face, edge, or vertex

    // Extra Information
    //TODO: Add Orientation Flag
    std::shared_ptr<PFace> mate;
};

class Face: public Entity
{
public:
    std::shared_ptr<PFace> pface;
    std::shared_ptr<Loop> loop;
    //TODO: Add Surface Geometry
};

class Loop: public Entity
{
public:
    std::shared_ptr<Face> face;
    std::shared_ptr<Loop> next;
    std::shared_ptr<PEdge> pedge;
};

class PEdge: public Entity
{
public:
    std::shared_ptr<Loop> loop;
    std::shared_ptr<Entity> child; // Can be a edge, or vertex

    // Extra Information
    //TODO: Add Orientation Flag
    std::shared_ptr<PVertex> pvertex; // Start pvertex
    std::shared_ptr<PEdge> looped_prev;
    std::shared_ptr<PEdge> looped_next;
    std::shared_ptr<PEdge> radial_prev;
    std::shared_ptr<PEdge> radial_next;
};

class Edge: public Entity
{
public:
    std::shared_ptr<Entity> parent; // Can be a partial face, or partial edge
    std::shared_ptr<PVertex> pvertices[2];
    //TODO: Add Curve Geometry
};

class PVertex: public Entity
{
public:
    std::shared_ptr<Edge> edge;
    std::shared_ptr<PVertex> next; //TODO: Evaluate if we actually need this
    std::shared_ptr<Vertex> vertex;
};

class Vertex: public Entity
{
public:
    std::shared_ptr<Entity> parent; // Can be a partial face, partial edge,
                                    // or partial vertex
    //TODO: Add Point Geometry
};

}