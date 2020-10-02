#include "TemporaryMeshManager.h"

#include "Mesh.h"
#include "Polyline.h"
#include <sstream>
#include <utility>

namespace Nome::Scene
{

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
}

void CTemporaryMeshManager::AddFace(const std::vector<std::string>& facePoints)
{
    // 10/1: I am using a dummy mesh instead of the testMesh because the faces NEED To be connected in a single mesh!!!!!!!!

    const std::string faceName = "f" + std::to_string(FaceCounter);
    TAutoPtr<CFace> face = new CFace(faceName);
    Scene->AddEntity(tc::static_pointer_cast<CEntity>(face)); // Add face as an entity. Technically, this step can be skipped because we are directly adding it to a single mesh below
    face->SetPointSourceNames(Scene, facePoints);
    TAutoPtr<CMesh> dummyMesh = new CMesh("Added" + faceName);
    dummyMesh->Faces.Connect(face->Face); 
    Scene->AddEntity(tc::static_pointer_cast<CEntity>(dummyMesh)); //needs to instanciated as a mesh face, not just a face

    // added below two line to try to allow Add Face to work with temp mesh vertices
    auto sceneNode = Scene->GetRootNode()->CreateChildNode("Added" + faceName);
    auto entity = Scene->FindEntity("Added" + faceName); // needs to be the dummyMesh, instead of the face
    sceneNode->SetEntity(entity); // this doesn't work right now because you can't create an instance of a single face

    FaceCounter += 1;
}

void CTemporaryMeshManager::AddPolyline(const std::vector<std::string>& facePoints)
{
    std::vector<std::string> currPoints =  std::vector<std::string>(facePoints.begin() + polyline_prev_num_points, facePoints.end());
    CSceneNode* TempPolylineNode = nullptr;
    if (!TempPolylineNode)
        TempPolylineNode = Scene->GetRootNode()->CreateChildNode("__tempPolylineNode"
                                                          + std::to_string(num_polylines));
    else
        TempPolylineNode->SetEntity(nullptr);
    
    TempPolyline = new CPolyline("__tempPolyline." + std::to_string(num_polylines)); 
    TempPolylineNode->SetEntity(TempPolyline);
    TempPolyline->SetPointSourceNames(Scene, currPoints); 
    TempPolyline->SetClosed(false); // Hardcoding the closed bool to true. Change in the future.
    num_polylines += 1;
    polyline_prev_num_points += currPoints.size();
}

std::string CTemporaryMeshManager::CommitTemporaryMesh(AST::CASTContext& ctx,
                                                       const std::string& entityName,
                                                       const std::string& nodeName)
{
    
    if (!TempMesh || !TempMeshNode)
        return "";

    if (!Scene->RenameEntity("__tempMesh", entityName))
        throw std::runtime_error("Cannot rename the temporary mesh, new name already exists");
    if (!TempMeshNode->SetName(nodeName))
        throw std::runtime_error("Cannot rename the scene node to the desired name");
    


    auto* meshCmd = TempMesh->SyncToAST(ctx, true);
    SourceMgr->AppendCmdEndOfFile(meshCmd);
    auto* instanceCmd = TempMeshNode->BuildASTCommand(ctx);
    SourceMgr->AppendCmdEndOfFile(instanceCmd);

    TempMesh = nullptr;
    TempMeshNode = nullptr;
    return "";
}

}
