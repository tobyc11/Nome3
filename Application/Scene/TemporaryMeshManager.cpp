#include "TemporaryMeshManager.h"

#include "Mesh.h"
#include "Polyline.h"
#include <sstream>
#include <utility>

// Steven's Add Point
#include "ASTSceneAdapter.h"
#include <map>

namespace Nome::Scene
{


std::string CTemporaryMeshManager::AddPoint(std::vector<std::string> pos)
{
    std::cout << pos[0] << " " <<  pos[1] << " " << pos[2] << "TempMesh AddPoint DEBUG" << std::endl;
    std::string pointName = "__tempPoint" + std::to_string(num_points);                     
    AST::ACommand* pointCmd =
        new AST::ACommand(new AST::CToken("point", 0, 1), new AST::CToken("endpoint", 0, 1));
    AST::AIdent* tempPointIdent = new AST::AIdent(new AST::CToken(pointName, 0, 1));
    AST::AVector* posAExpr =
        new AST::AVector(new AST::CToken("(", 0, 1), new AST::CToken(")", 0, 1));

    AST::ANumber* num1A = new AST::ANumber(new AST::CToken(pos.at(0), 0, 2));
    AST::ANumber* num2A = new AST::ANumber(new AST::CToken(pos.at(1), 0, 2));
    AST::ANumber* num3A = new AST::ANumber(new AST::CToken(pos.at(2), 0, 2));

    posAExpr->AddChild(num1A);
    posAExpr->AddChild(num2A);
    posAExpr->AddChild(num3A);

    pointCmd->PushPositionalArgument(tempPointIdent);
    pointCmd->PushPositionalArgument(posAExpr);

    TAutoPtr<CEntity> tempPoint = CASTSceneAdapter::MakeEntity(pointCmd->GetCommand(), pointName);
    tempPoint->GetMetaObject().DeserializeFromAST(*pointCmd, *tempPoint);
    Scene->AddEntity(tempPoint);

    std::vector<std::string> points = { pointName };
    CSceneNode* TempPolylineNode = nullptr;
    if (!TempPolylineNode)
        TempPolylineNode = Scene->GetRootNode()->CreateChildNode("__tempPolylineNodePoint"
                                                                 + std::to_string(num_points));
    else
        TempPolylineNode->SetEntity(nullptr);

    TempPolylinePoint = new CPolyline("__tempPolylinePointNode." + std::to_string(num_points));
    TempPolylineNode->SetEntity(TempPolylinePoint);
    TempPolylinePoint->SetPointSourceNames(Scene, points);
    TempPolylinePoint->SetClosed(false);
    num_points += 1;
    polyline_prev_num_points += 1;

    std::string cmd = "\npoint " + pointName + " ( " + pos.at(0) + " " + pos.at(1) + " " + pos.at(2)
        + ") endpoint\npolyline __tempPolylineNodePoint" + std::to_string(num_points) + " ( "
        + pointName + " ) endpolyline\ninstance __inst__tempPolylineNodePoint"
        + std::to_string(num_points) + " __tempPolylineNodePoint" + std::to_string(num_points)
        + " endinstance\n";
    pointMap.insert(std::pair<std::string, std::string>(pointName, cmd));
    // SourceMgr->AppendText(
    //     "\npoint " + pointName + " ( " + pos.at(0) + " " + pos.at(1) + " " + pos.at(2)
    //     +") endpoint\npolyline __tempPolylineNodePoint" + std::to_string(num_points)
    //     + " ( " + pointName +" ) endpolyline\ninstance __inst__tempPolylineNodePoint"
    //     + std::to_string(num_points) + " __tempPolylineNodePoint" + std::to_string(num_points) +
    //     " endinstance\n");

    return pointName;
}


// Warning: this function is poorly implemented. Need to fix in the future
void CTemporaryMeshManager::RemoveFace(const std::vector<std::string>& faceNames)
{
    Scene->Update();
    Scene->ForEachSceneTreeNode([&](Scene::CSceneTreeNode* node) {
        auto* entity = node->GetInstanceEntity(); 
        if (!entity)
            entity = node->GetOwner()->GetEntity();
        if (auto* mesh = dynamic_cast<Scene::CMeshInstance*>(entity))
        {
            auto faceVertNames =
                mesh->RemoveFace(faceNames); // silently do nothing if mesh doesn't have face points
            if (faceVertNames.size() != 0)
                removedfaceNames.push_back(mesh->GetSceneTreeNode()->GetPath());
        }
    });
}

// This function should roughly mirror the structure of CASTSceneBuilder::VisitFace
void CTemporaryMeshManager::AddFace(const std::vector<std::string>& facePoints)
{
    // if any of the selected facePoints contain a TempMesh name, then we should build a new TempMesh
    bool createNewMesh = false;
    for (auto facePoint : facePoints)
    {
        if (facePoint.find("TempMesh") != std::string::npos || addedMeshes.size() == 0) 
            createNewMesh = true;
    }
    if (createNewMesh)
    { 
        const std::string meshName = "TempMesh" + std::to_string(FaceCounter);
        const std::string faceName = meshName + "nnew" + std::to_string(FaceCounter);
        TAutoPtr<CFace> face = new CFace(faceName);
        Scene->AddEntity(tc::static_pointer_cast<CEntity>(face)); 
        face->SetPointSourceNames(Scene, facePoints); 
        TAutoPtr<CMesh> dummyMesh = new CMesh(meshName);
        dummyMesh->Faces.Connect(face->Face);
        Scene->AddEntity(tc::static_pointer_cast<CEntity>(
            dummyMesh)); // needs to instanciated as a mesh face, not just a face
        auto sceneNode = Scene->GetRootNode()->CreateChildNode("inst" + meshName);
        auto entity = Scene->FindEntity(meshName); 
        sceneNode->SetEntity(entity); 
        addedSceneNodes.push_back(sceneNode);
        addedMeshes.push_back(dummyMesh);
        FaceCounter += 1;
    }
    else
    {
        // the facepoints do not contain a tempmesh point and is not the first mesh  being added"
        // in this case, we append the added face name to the dummyMesh
        auto dummyMesh = addedMeshes.back(); //  get the most recently added mesh
        auto sceneNode = addedSceneNodes.back(); 
        auto meshName = dummyMesh->GetName();
        const std::string faceName = meshName + "." + "new" + std::to_string(FaceCounter);
        TAutoPtr<CFace> face = new CFace(faceName);
        face->SetPointSourceNames(Scene, facePoints);
        Scene->AddEntity(tc::static_pointer_cast<CEntity>(face));
        dummyMesh->Faces.Connect(face->Face);
        FaceCounter += 1;
    }
}

void CTemporaryMeshManager::AddPolyline(const std::vector<std::string>& points)
{
    const std::string polyName = "TempPoly" + std::to_string(num_polylines);
    // std::vector<std::string> currPoints =  std::vector<std::string>(facePoints.begin() +
    // polyline_prev_num_points, facePoints.end());
    TAutoPtr<CPolyline> polyline = new CPolyline(polyName);
    polyline->SetClosed(false); // Hardcoding the closed bool to false. Change in the future.
    polyline->SetPointSourceNames(Scene, points);
    Scene->AddEntity(tc::static_pointer_cast<CEntity>(polyline));
    auto sceneNode = Scene->GetRootNode()->CreateChildNode("inst" + polyName);
    auto entity = Scene->FindEntity(polyName);
    sceneNode->SetEntity(entity);

    addedSceneNodes.push_back(sceneNode);
    addedMeshes.push_back(polyline);
    num_polylines += 1;
}

// TODO: Make it so can interactively click on this polyline and add a point there. The polyline will automatically dissapear afterwards.
void CTemporaryMeshManager::AddInteractivePolyline(const std::vector<std::string>& points)
{
    const std::string polyName = "TempInteractivePoly" + std::to_string(num_polylines);
    TAutoPtr<CPolyline> polyline = new CPolyline(polyName);
    polyline->SetClosed(false); // Hardcoding the closed bool to false. Change in the future.
    polyline->SetPointSourceNames(Scene, points);
    Scene->AddEntity(tc::static_pointer_cast<CEntity>(polyline));
    auto sceneNode = Scene->GetRootNode()->CreateChildNode("inst" + polyName);
    auto entity = Scene->FindEntity(polyName);
    sceneNode->SetEntity(entity);
    addedSceneNodes.push_back(sceneNode);
    addedMeshes.push_back(polyline);
    num_polylines += 1;
}

// Highlight selected edge by adding temp polyline
void CTemporaryMeshManager::SelectOrDeselectPolyline(const std::vector<std::string>& points)
{
    auto searchpoint0 = points[0].substr(0, points[0].find("._"));
    auto searchpoint1 = points[0].substr(0, points[0].find("._"));
    bool alreadySelected = false;
    std::string removeName;
    for (auto name : AddedTempPolylineNodeNames)
    {
        std::cout << "name: " + name << std::endl;
        if (searchpoint0.find(name) != std::string::npos
            && searchpoint1.find(name) != std::string::npos)
        {
            removeName = name;
            alreadySelected = true;
        }
    }

    const std::string polyName = "SELECTED" + points[0]
        + points[1]; // name is just the edge vert names concatenated Randyt added SELECTED on 11/21

    if (alreadySelected) // Deselect
    {
        // const std::string polyName = points[0] + points[1]; // name is just the edge vert names
        // concatenated CSceneNode *temp;
        Scene->ForEachSceneTreeNode([&](Scene::CSceneTreeNode* node) {
            if (node->GetOwner()->GetName() == removeName)
            { // removeName includes the "inst" as prefix
                std::cout << removeName << std::endl;
                node->GetOwner()->SetEntity(nullptr);
            }
        });
    }
    else // Select
    {
        TAutoPtr<CPolyline> polyline = new CPolyline(polyName);
        polyline->SetClosed(false); // Hardcoding the closed bool to false. Change in the future.
        polyline->SetPointSourceNames(
            Scene,
            points); // point names includes entity names for some reason ... it's because these two
                     // points  start w/ ".cube1.front.p1.cube1.front.p4" and end with ._
        Scene->AddEntity(tc::static_pointer_cast<CEntity>(polyline));
        auto sceneNode = Scene->GetRootNode()->CreateChildNode("inst" + polyName);
        std::cout << "inst" + polyName << std::endl;
        AddedTempPolylineNodeNames.push_back("inst" + polyName);
        auto entity = Scene->FindEntity(polyName);
        sceneNode->SetEntity(entity);
    }
}

std::string CTemporaryMeshManager::CommitChanges(AST::CASTContext& ctx)
{
    auto* blankLineNode1 = ctx.Make<AST::ACommand>(ctx.MakeToken(" "), ctx.MakeToken(" "));
    auto* blankLineCmd1 = blankLineNode1;
    SourceMgr->AppendCmdEndOfFile(blankLineCmd1);

    auto* poundNode =
        ctx.Make<AST::ACommand>(ctx.MakeToken("###########################"), ctx.MakeToken(" "));
    auto* dummyPoundCmd = poundNode;
    SourceMgr->AppendCmdEndOfFile(dummyPoundCmd);

    auto* blankLineNode2 = ctx.Make<AST::ACommand>(ctx.MakeToken(" "), ctx.MakeToken(" "));
    auto* blankLineCmd2 = blankLineNode2;
    SourceMgr->AppendCmdEndOfFile(blankLineCmd2);

    if (!removedfaceNames.empty())
    {
        auto* polylineNode =
            ctx.Make<AST::ACommand>(ctx.MakeToken("# REMOVED"), ctx.MakeToken("ENDREMOVED"));
        //  polylineNode->PushPositionalArgument(
        //     ctx.MakeIdent(GetName())); // 1st positional arg is name
        // 2nd positional arg is point ident vector

        std::vector<AST::AExpr*> identList;
        for (const auto& pointName : removedfaceNames)
            identList.push_back(ctx.MakeIdent(pointName));
        polylineNode->PushPositionalArgument(ctx.MakeVector(identList));
        auto* dummyremoveCmd = polylineNode;
        SourceMgr->AppendCmdEndOfFile(dummyremoveCmd);
    }

    size_t numAdded = addedMeshes.size();
    for (int i = 0; i < numAdded; i++)
    {
        auto addedMesh = addedMeshes[i];
        auto addedNode = addedSceneNodes[i];
        // Randy in progress. save polyline to .nom file.
        if (dynamic_cast<CPolyline*>(addedMesh)) // if it's actually a polyline
        {
            auto* meshCmd = addedMesh->SyncToAST(ctx, true);
            SourceMgr->AppendCmdEndOfFile(meshCmd);
            auto* instanceCmd = addedNode->BuildASTCommand(ctx);
            SourceMgr->AppendCmdEndOfFile(instanceCmd);
        }
        else // else, it's a mesh command
        {
            auto* meshCmd = addedMesh->SyncToAST(ctx, true);
            SourceMgr->AppendCmdEndOfFile(meshCmd);
            auto* instanceCmd = addedNode->BuildASTCommand(ctx);
            SourceMgr->AppendCmdEndOfFile(instanceCmd);
        }
    }

    addedSceneNodes.clear();
    addedMeshes.clear();
    /*
    TempMesh = nullptr;
    TempMeshNode = nullptr;
    */

     // Steven's Add Point
    for (std::map<std::string, std::string>::iterator p = pointMap.begin(); p != pointMap.end();
         ++p)
    {
        SourceMgr->AppendText(p->second);
    }
    pointMap.clear();

    return "";
}

}