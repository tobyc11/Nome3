/**
 * @author Andy Wang, UC Berkeley.
 * Copyright 2016 reserve.
 * UC Berkeley, Slide_2016 project.
 * Advised by Prof. Sequin H. Carlos.
 */

#ifndef __MESH_H__
#define __MESH_H__

#include <string>
#include <unordered_map>
#include <vector>
#include "DataStructureFace.h"
//#include "transformation.h"
//#include "utils.h"
class Parameter;
class Group;

using namespace std;
//using namespace glm;

//////////////////////////////////////////////////////////////////////
// Mesh Class -- A MESH is a collection of polygon facets
class Mesh
{
public:
    /* A list of all vertices in this mesh. */
    vector<Vertex*> vertList;
    /* A list of all facets in this mesh.*/
    vector<Face*> faceList;

    // Randy added this. Easy way to access Vertex objects in Mesh.cpp. Don't need to traverse vertList
    unordered_map<string, Vertex*> nameToVert;

    /* This is an auxillary table to build a mesh, matching edge to vertex.*/
    unordered_map<Vertex*, vector<Edge*>> edgeTable;
    Mesh(int type = 0);
    /**
     * @brief addVertex: Add one vertex to this Mesh.
     * @param v, the vertex to be added in.
     */
    void addVertex(Vertex* v);
    /**
     * @brief addVertex: Add one vertex to this Mesh,
     * the position of the vertex is made of an expression.
     * @param v, the vertex to be added in.
     * @param expr, the expresssions for the vertex.
     */
//    void addVertex(Vertex* v, vector<string>& expr);
    /**
     * @brief deleteVertex: Delete one vertex from this Mesh.
     * If vertex is not in this mesh, do nothing.
     * @param v: a pointer to the vertex that we want to delete.
     */
    void deleteVertex(Vertex* v);
    /**
     * @brief Add one edge v1-v2 to this Mesh.
     * @param v1, v2: the two vertices of this edge.
     * If it already exists, then return the existing edge.
     */
    Edge* createEdge(Vertex* v1, Vertex* v2);
    /**
     * @brief Find one edge v1-v2 in this Mesh.
     * @param v1, v2: the two vertices of this edge.
     * If it does not exists, then return NULL.
     */
    Edge* findEdge(Vertex* v1, Vertex* v2);
    /**
     * @brief deleteEdge Delete edge v1-v2 in this Mesh.
     * @param v1, v2: the two vertices of this edge.
     */
    void deleteEdge(Edge* edge);
    /**
     * @brief Add a triangle face to a mesh, with three vertices.
     * @param v1, v2, v3 are the three vertices of the face.
     */
    void addTriFace(Vertex* v1, Vertex* v2, Vertex* v3);
    // Add a quad face to a mesh, with three vertices.
    // @param v1, v2, v3, v4 are the four vertices of the face.
    void addQuadFace(Vertex* v1, Vertex* v2, Vertex* v3, Vertex* v4);
    // Add a arbitrary polygon face to a mesh, with three vertices.
    // @param vertices is a list of consequtive vertices of the face.
    Face* addPolygonFace(vector<Vertex*> vertices, bool reverseOrder = false); // Randy changed this on 1/22
    /**
     * @brief delelteFace: Delete a face from this mesh.
     * @param face: the face to be deleted.
     */
    void deleteFace(Face* face);
    /**
     * @brief drawMesh:  Draw a mesh in OpenGL
     * @param startIndex: The starting index of drawing polygon name.
     * @param smoothShading: Indicate if we are in smooth shading or flat shading.
     */
    /**
     * @brief makeCopy: Make a copy of current mesh.
     * @return The copied mesh.
     */
    Mesh makeCopy(string copy_mesh_name = "");
    /**
     * @brief makeCopyForTransform: Make a copy of current mesh.
     * It is used for future transformation.
     * @return The copied mesh in order to transform.
     */
  //  Mesh makeCopyForTransform();
    /**
     * @brief makeCopyForTempMesh: Make a copy of the temp mesh from canvas.
     * @param copy_mesh_name: the name of the new copy mesh.
     * @return The copied mesh.
     */
  //  Mesh makeCopyForTempMesh(string copy_mesh_name);
 //   void updateCopyForTransform();
    /**
     * @brief transform: Transform this mesh.
     * @param t: The transformation for this mesh.
     */
  //  void transform(Transformation* t);
  //  void transform(mat4 matrix);
  //  mat4 transformToTop();
    //void drawMesh(int startIndex, bool smoothShading);
    // Draw the selected vertices in OpenGL
   // void drawVertices();
    // Build Boundary Pointers for Mesh.
    void buildBoundary();
    // Reverse the effect of buildBoundary function on some vertices. */
    void setBoundaryEdgeToNull(Vertex* v);
    // Compute the vertex normals for every face and vertex of the mesh.
    void computeNormals();
    // Return the list of edges on the boarders
    vector<Edge*> boundaryEdgeList();
    /* The color of this mesh.*/
    //QColor color;
    /* Check if this mesh if empty. */
    bool isEmpty();
    /* Set color of this mesh*/
    //void setColor(QColor color);
    /**
     * @brief clear: clear the current mesh.
     */
    void clear();
    /**
     * @brief clear: clear the current mesh and delete the vertices and faces.
     */
    void clearAndDelete();
    /* Indicator of whether user sets the color of this mesh.*/
    bool user_set_color;
    /* transformation matrix to go up one level.*/
    //vector<Transformation> transformations_up;
    /* Add one transformation to this mesh of going up one level. */
   // void addTransformation(Transformation new_transform);
    /* Reset the transformations to this mesh of going up one level. */
    //void setTransformation(vector<Transformation>);
    /* The name of this mesh. */
    string name;
    /* Update the value of all elements made by expression. */
    //void updateMesh();
    /* A map of vertex ID to its position expression. */
    unordered_map<int, vector<string>> idToExprs;
    /* A pointer to the global parameter. */
    unordered_map<string, Parameter>* params;
    /* Set the global parameter pointer for this mesh. */
    void setGlobalParameter(unordered_map<string, Parameter>* params);
    /* The paraent group of this mesh. */
    Group* parent;
    /**
     * Type of this mesh.
     * 0: A general mesh.
     * 1: A Funnel.
     * 2: A Tunnel.
     * 3: A rim line.
     * 99: Consolidated mesh or Temporary mesh.
     */
    int type;
    /* Parameters used by funnel. */
    int n;
    float ro;
    float ratio;
    float h;
    string n_expr;
    string ro_expr;
    string ratio_expr;
    string h_expr;
    //void makeFunnel();
    //void makeTunnel();
    //void makeTunnel2();
    //void updateFunnel();
    //void updateTunnel();
    //void updateFunnel_n();
    //void updateTunnel_n();
    //void updateFunnel_ro_ratio_or_h();
    //void updateTunnel_ro_ratio_or_h();
    //int setFunnelParameterValues(string, int);
    //void setTunnelParameterValues(string, int);
    vector<Parameter*> influencingParams;
    /* Add a parameter that influence this funnel. */
    void addParam(Parameter*);
    /* The pointer to the copied mesh before transformation. */
    Mesh* before_transform_mesh;
    /* Find a vertex in this mesh given its name. */
    Vertex* findVertexInThisMesh(string name);
    /* Find a face in this mesh given its name. And delete this face.
     * return true if it is in this mesh. */
    bool deleteFaceInThisMesh(string name);
    /* Indicator that this group is in editing mode.
     * So we can't change the hyper parameters of this mesh,
     * e.g. the parameter n of the mesh.
     */
    bool in_editing_mode;
    /* For the face deletion for consolidate mesh.*/
    bool isConsolidateMesh;
    /* Update the vertList after we delete from consolidate mesh.*/
    void updateVertListAfterDeletion();
};

// @param p1, p2, p3 are positions of three vertices,
// with edge p1 -> p2 and edge p2 -> p3.
tc::Vector3 getNormal3Vertex(tc::Vector3 p1, tc::Vector3 p2, tc::Vector3 p3);

#define PI (glm::pi<float>())
#define VERYSMALLVALUE 0.001

#endif // __MESH_H__
