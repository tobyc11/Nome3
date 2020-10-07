#include "TemporaryMeshManager.h"

#include "Mesh.h"
#include "Polyline.h"
#include <sstream>
#include <utility>

namespace Nome::Scene
{

// Randy note: after 10/1, this is not used anymore
/*
void CTemporaryMeshManager::ResetTemporaryMesh()
{
    if (!TempMeshNode)
        TempMeshNode = Scene->GetRootNode()->CreateChildNode("__tempMeshNode");
    else
        TempMeshNode->SetEntity(nullptr);

    if (!TempPolylineNode)
        TempPolylineNode = Scene->GetRootNode()->CreateChildNode("__tempPolylineNode");
    else
        TempPolylineNode->SetEntity(nullptr);



    // Make entity and its corresponding scene node
    //Commented out on 9/30. Instead of creating a single tempMesh node, I'm now creating a tempMesh (dummyMesh) for each face added
    Scene->RemoveEntity("__tempMesh", true);
    TempMesh = new CMesh("__tempMesh");
    Scene->AddEntity(TempMesh);
    FaceCounter = 0;

    Scene->RemoveEntity("__tempPolyline", true);
    TempPolyline = new CPolyline("__tempPolyline");
    Scene->AddEntity(TempPolyline);
    num_polylines = 0;

    TempMeshNode->SetEntity(TempMesh);
    TempPolylineNode->SetEntity(TempPolyline);
}*/


void CTemporaryMeshManager::RemoveFace(const std::vector<std::string>& facePoints)
{
    // create a set containing all the facepoint locations: CMeshImpl::Point(pos.x, pos.y, pos.z) aftrer finding the points 
   // work in progress. Not sure if TempMeshManager is best location.
    //Scene->Update();
    //std::vector<std::vector<float>> allpoints; //vector of point coordinates
    //for (auto name : facePoints)
    //{
    //    std::vector<float> coordinate; 
    //    auto  point = dynamic_cast<Scene::CPoint*>(Scene->FindEntity(name).Get()); // get point entity. Reminder, points are entities too
    //    coordinate.push_back(point->X.GetValue(99999));
    //    coordinate.push_back(point->Y.GetValue(99999));
    //    coordinate.push_back(point->Z.GetValue(99999));
    //    allpoints.push_back(coordinate);
    //}

    //tc::TAutoPtr<Scene::CMesh> test = new Scene::CMesh("globalMerge"); 
    //Scene->ForEachSceneTreeNode([&](Scene::CSceneTreeNode* node) {
    //    auto* entity = node->GetInstanceEntity(); // this is non-null if the entity is instantiable like a torus knot or polyline
    //    if (!entity) // if it's not instantiable, like a face, then get the entity associated with it
    //        entity = node->GetOwner()->GetEntity();
    //    if (auto* mesh = dynamic_cast<Scene::CMeshInstance*>(entity))
    //    {
    //        auto& openmesh = mesh->GetMeshImpl();
    //        // https://mcoder.cc/2019/07/06/half_edge_data_structure/
    //        
    //        for (auto fi = openmesh.faces_begin(); fi != openmesh.faces_end(); ++fi) {
    //            auto currfacehandle = fi.handle();
    //            std::vector<CMeshImpl::VertexHandle> verts;
    //            bool foundface = true;
    //            for (auto verthandle : openmesh.fv_range(*fi))
    //            { // iterate through all the vertices on this face
    //                std::vector<float> tempcoordinate;
    //                float X = openmesh.point(verthandle)[0];
    //                float Y = openmesh.point(verthandle)[1];
    //                float Z = openmesh.point(verthandle)[2];
    //                tempcoordinate.push_back(X);
    //                tempcoordinate.push_back(Y);
    //                tempcoordinate.push_back(Z);
    //                for (auto coordinate : allpoints) {
    //                    if (coordinate == tempcoordinate) {
    //                        openmesh.request_face_status(); //instead just create a new scene without this face
    //                        openmesh.
    //                    }
    //                }
    //                if (vert is not inide the set of vertex location)
    //                    foundface = false
    //            }
    //            if !noit
    //            {
    //                remove face
    //            }
    //        }
    //            ...; // do something with *f_it, f_it->, or *f_it

    //    }    
    //});

}


void CTemporaryMeshManager::AddFace(const std::vector<std::string>& facePoints)
{
    const std::string meshName = "Placeholder" + std::to_string(FaceCounter);
    const std::string faceName = meshName + "nnew" + std::to_string(FaceCounter);
    TAutoPtr<CFace> face = new CFace(faceName);
    Scene->AddEntity(tc::static_pointer_cast<CEntity>(face)); // Add face as an entity. Technically, this step can be skipped because we are directly adding it to a single mesh below
    face->SetPointSourceNames(Scene, facePoints);
    TAutoPtr<CMesh> dummyMesh = new CMesh(meshName);
    dummyMesh->Faces.Connect(face->Face); 
    Scene->AddEntity(tc::static_pointer_cast<CEntity>(dummyMesh)); //needs to instanciated as a mesh face, not just a face

    auto sceneNode = Scene->GetRootNode()->CreateChildNode("inst" + meshName);
    auto entity = Scene->FindEntity(meshName); // entity needs to be the dummyMesh, instead of the face as we can't instanciate a face rn
    sceneNode->SetEntity(entity); // this doesn't work right now because you can't create an instance of a single face
   
    
    addedSceneNodes.push_back(sceneNode);
    addedMeshes.push_back(dummyMesh);
    FaceCounter += 1;
}

void CTemporaryMeshManager::AddPolyline(const std::vector<std::string>& facePoints)
{
    const std::string polyName = "PlaceholderPoly" + std::to_string(num_polylines);
    //std::vector<std::string> currPoints =  std::vector<std::string>(facePoints.begin() + polyline_prev_num_points, facePoints.end());
    TAutoPtr<CPolyline> polyline = new CPolyline(polyName);
    polyline->SetClosed(false); // Hardcoding the closed bool to true. Change in the future.
    polyline->SetPointSourceNames(Scene, facePoints);
    Scene->AddEntity(tc::static_pointer_cast<CEntity>(polyline));
    auto sceneNode = Scene->GetRootNode()->CreateChildNode("inst" + polyName);
    auto entity = Scene->FindEntity(polyName);
    sceneNode->SetEntity(entity);

    addedSceneNodes.push_back(sceneNode);
    addedMeshes.push_back(polyline);
    num_polylines += 1;
}

std::string CTemporaryMeshManager::CommitChanges(AST::CASTContext& ctx)
{
    for (auto addedMesh : addedMeshes)
    {

        /* Randy in progress. save polyline to .nom file.
        if (dynamic_cast<CPolyline*>(addedMesh)) // if it's actually a polyline
        {
            auto* meshCmd = addedMesh->SyncToAST(ctx, true);
            SourceMgr->AppendCmdEndOfFile(meshCmd);
        } else...*/
 
        auto* meshCmd = addedMesh->SyncToAST(ctx, true);
        SourceMgr->AppendCmdEndOfFile(meshCmd);
        
    }

    for (auto addedNode : addedSceneNodes)
    {
        auto* instanceCmd = addedNode->BuildASTCommand(ctx);
        SourceMgr->AppendCmdEndOfFile(instanceCmd);
    }
    /*
    TempMesh = nullptr;
    TempMeshNode = nullptr;
    */
    return "";
}

}
