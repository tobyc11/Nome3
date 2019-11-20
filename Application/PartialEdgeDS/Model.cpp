//
// Created by 殷子欣 on 2019/10/14.
//

#include "Model.h"
#include "Region.h"
#include "Vertex.h"
#include "Edge.h"
#include "Face.h"
#include "PEdge.h"
#include "PFace.h"
#include <vector>

namespace Nome::PartialEdgeDS
{

Model::~Model()
{
    if (prev != NULL) { prev->next = next; }
    if (next != NULL) { next->prev = prev; }
}


bool Model::killVertex(const Vertex *&vertex) {
    if (vertex->type == EType::PVERTEX) { return false; }
    delete(vertex);
    return true;
}

bool Model::killEdge(const Edge *&edge)
{
    if (edge->type == EType::PEDGE) { return false; }
    edge->killChildren();
    delete(edge);
    return true;
}

bool Model::killFace(const Edge *&edge, const Face *&face)
{
    std::vector<Edge *> edges = face->getEdges();
    if (std::find(edges.begin(), edges.end(), edge) == edges.end()) { return false; }

    edge->killChildren();
    delete(edge);

    PFace *pFace = face->pFace;

    for (std::vector<Edge *>::iterator it = edges.begin(); it != edges.end(); it++)
    {
        const Shell *&shell, const PFace *&next, const PFace *&prev, const Entity &*child, const EType &type
        ((PEdge *)(*it)->parent)->child = NULL;
        PFace *newPFace = new PFace(pFace->shell, pFace->next, pFace->prev, (*it), EType::EDGE);
        (*it)->type = EType::PFACE;
        (*it)->parent = newPFace;
        pFace = newPFace;
    }
    face->killChildren();
    delete(face);
    return true;
}

void Model::killChildren() {

    Region *tempRegion = region, *deleteRegion;
    while (tempRegion != NULL)
    {
        tempRegion->killChildren();
        tempRegion->model = NULL;
        deleteRegion = tempRegion;
        tempRegion = tempRegion->next;
        delete(deleteRegion);
    }

}


bool cutFaceByTwoVertexes(const std::string &vertexID1, const std::string &vertexID2)
{
    return true;
}

std::vector<Region *> getRegions() const {
    std::vector<Region *> regions;
    Region *tempRegion = region;

    while (tempRegion != NULL)
    {
        regions.push_back(tempRegion);
        tempRegion = tempRegion->next;
    }
    return regions;
    /**
     * TODO: Implement Merge Models for merging A and B
     *  - First, Merge HashTables, get list of new B ids
     *  - Apply changes to B for ids at Faces, Edges, Vertices
     *  - Append Region(s) from B to A
     *  - Null out *region
     *  - Kill B -> return pointer to next guy after B
     *  - Stitch to previous
     *  - Free B
    */

void Model::mergeModel(Model *bModel)
{
    //First, Merge HashTables, get list of new B ids
    auto keysToChangeInB = Entity::mapper.mergeModels(uid, bModel->uid);

    //Apply changes to B for ids at Faces, Edges, Vertices
    changeKeysInModel(keysToChangeInB);

    //Append Region(s) from B to A
    if (region == NULL)
    {
        region = bModel->region;
    }
    else
    {
        Region* current = region;
        while(current->next != NULL) {
            current = current->next;
        }
        current->next = bModel->region;
    }

    //Null out *region
    bModel->region = NULL;

    //Kill B, then stitch to previous
    Model *beforeB = bModel->prev;
    Model *afterB = bModel->killModel();
    beforeB->next = afterB;

    //Delete B
    bModel->next = NULL;    //Prevent it from deleting the rest of the models
    delete bModel;
}

void Model::changeKeysInModel(std::map<std::pair<EGType, u_int64_t>, u_int64_t> *keyMap)
{

}

} /* namespace Nome::Scene::PartialEdgeDS */
