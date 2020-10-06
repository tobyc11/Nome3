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


void CTemporaryMeshManager::DeleteFace(const std::vector<std::string>& facePoints)
{
   // work in progress
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
