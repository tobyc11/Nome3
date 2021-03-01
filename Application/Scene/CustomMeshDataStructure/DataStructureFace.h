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
    Face(std::vector<Vertex*> vertices); // Randy changed Vertex to Vertex*
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
    std::vector<int> v_ids;
    std::array<float, 3> color; // QColor color; 1/28 randy updated this to std::aray<float, 3> so easily usable in DataStructureMeshToQGeometry
    std::vector<Vertex*> vertices; // Randy noticed this wasn't being used before? Randy changed it from Vertex to Vertex*
    /* Indicate if this face has user defined color. */
    bool user_defined_color;

    string surfaceName; // Randy added this
};

#endif // __FACE_H__
