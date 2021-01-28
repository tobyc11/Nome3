/**
 * @author Andy Wang, UC Berkeley.
 * Copyright 2016 reserve.
 * UC Berkeley, NOME project.
 * Advised by Prof. Sequin H. Carlos.
 */

#ifndef __FACE_H__
#define __FACE_H__

#include "DataStructureEdge.h"
#include "DataStructureMesh.h" // TODO: fix this randy, maybe need to use DataStructureMesh
#include "DataStructureVertex.h"
//#include <QColor>

#include <vector>

using namespace std;

/**
 * @brief The Face class. The face class build for winged-
 * edge data structure. A face is constructed by a seqence of
 * vertices/edges.
 */
class Face
{
public:
    Face();
    Face(std::vector<Vertex> vertices);
    /* The face normal.*/
    tc::Vector3 normal;
    /* Pointer to one edge in this face.*/
    Edge* oneEdge;
    /* Pointer to the face point in subdivision. */
    Vertex* facePoint;
    /* Indicator of whether this face is selected.*/
    bool selected;
    /* FaceID, is the index from the global face list.*/
    int id;
    /* The name of this face.*/
    string name;
    /* The color of this face. */
    //QColor color;
    std::vector<Vertex> vertices;
    /* Indicate if this face has user defined color. */
    bool user_defined_color;
};

#endif // __FACE_H__
