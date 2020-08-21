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
    if (!TempMesh || !TempMeshNode)
        ResetTemporaryMesh();

    // This function should roughly mirror the structure of CASTSceneBuilder::VisitFace
    const std::string entityNamePrefix = "__tempMesh.";
    const std::string faceName = "f" + std::to_string(FaceCounter);

    TAutoPtr<CFace> face = new CFace(entityNamePrefix + faceName);
    face->SetPointSourceNames(Scene, facePoints);
    Scene->AddEntity(tc::static_pointer_cast<CEntity>(face));

    TempMesh->Faces.Connect(face->Face);
}

void CTemporaryMeshManager::AddPolyline(const std::vector<std::string>& facePoints)
{
    std::vector<std::string> currPoints =  std::vector<std::string>(facePoints.begin() + polyline_prev_num_points, facePoints.end());
    std::cout << "Inside Add Polyline" << std::endl;
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
