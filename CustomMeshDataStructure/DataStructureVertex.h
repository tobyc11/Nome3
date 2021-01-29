/**
 * @author Andy Wang, UC Berkeley.
 * Copyright 2016 reserve.
 * UC Berkeley, Slide_2016 project.
 * Advised by Prof. Sequin H. Carlos.
 */

#ifndef __VERTEX_H__
#define __VERTEX_H__


#include <string>
#include <unordered_map>
#include <vector>
#include <Vector3.h>

using namespace std;

class Edge;
class Mesh;
class Parameter;

/**
 * @brief The Vertex class. Build for vertex in winged-
 * edge data structure.
 */
class Vertex
{
public:
    Vertex();
    /**
     * @brief Vertex: Constructor of class Vertex.
     * @param x: x coordinate.
     * @param y: y coordinate.
     * @param z: z coordinate.
     * @param ID: The vertex ID.
     */
    Vertex(float x, float y, float z, unsigned long ID);
    Vertex(float x, float y, float z, string assignedName, unsigned long ID);
    /* The position. */
    tc::Vector3 position;
    /* Vertex normal. */
    tc::Vector3 normal;
    /* One out-going halfedge of this vertex.*/
    Edge* oneEdge;
    /* A tracking identifier. Generally the index from global vertex list.*/
    unsigned long ID;
    /* True if it is the end of any Mobius edge.*/
    bool onMobius;
    /* A pointer to the vertex point in subdivision.*/
    Vertex* vertexPoint;
    /* A pointer to the positive offset vertex in offsetting. */
    Vertex* posOffset;
    /* A pointer to the negative offset vertex in offsetting. */
    Vertex* negOffset;
    /* A pointer to its original vertex before transformation.
     * This is only used for a transformed mesh. */
    Vertex* before_transform_vertex;
    /* For the consolidated mesh and temp mesh. Find its own source vertex.*/
    Vertex* source_vertex;
    /* Indicator of whether this vertex is selected.*/
    bool selected;
    /* Indicator of whether this vertex is parametric. */
    bool isParametric;
    /* The string expression of its position x. */
    string x_expr;
    /* The string expression of its position y. */
    string y_expr;
    /* The string expression of its position z. */
    string z_expr;
    /* The name of this vertex.*/
    string name;
    /* Add a parameter that influence this funnel. */
    //void addParam(Parameter*);
    /* Parameters that influence this vertex. */
    //vector<Parameter*> influencingParams;
    /* A pointer to the global parameter. */
    //unordered_map<string, Parameter>* params;
    /* Set the parameters for this vertex. */
    //int setVertexParameterValues(string, int);
    /* Set the global parameter pointer for this vertex. */
    //void setGlobalParameter(unordered_map<string, Parameter>* params);
    /* Update the current position of this vertex.*/
    void update();
};

#endif // __VERTEX_H__
