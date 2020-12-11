#include "TemporaryMeshManager.h"

#include "Mesh.h"
#include "Polyline.h"
#include <sstream>
#include <utility>

namespace Nome::Scene
{

// Randy note: after 10/1, ResetTemporaryMesh is not used anymore.
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
    //Commented out on 9/30. Instead of creating a single tempMesh node, I'm now creating a tempMesh
(dummyMesh) for each face added Scene->RemoveEntity("__tempMesh", true); TempMesh = new
CMesh("__tempMesh"); Scene->AddEntity(TempMesh); FaceCounter = 0;
    Scene->RemoveEntity("__tempPolyline", true);
    TempPolyline = new CPolyline("__tempPolyline");
    Scene->AddEntity(TempPolyline);
    num_polylines = 0;
    TempMeshNode->SetEntity(TempMesh);
    TempPolylineNode->SetEntity(TempPolyline);
}*/

// Randy note: probably not good to place removeface here
void CTemporaryMeshManager::RemoveFace(const std::vector<std::string>& faceNames)
{
    Scene->Update();
    Scene->ForEachSceneTreeNode([&](Scene::CSceneTreeNode* node) {
        auto* entity = node->GetInstanceEntity(); // this is non-null if the entity is instantiable
                                                  // like a torus knot or polyline
        if (!entity)
            entity = node->GetOwner()->GetEntity();
        if (auto* mesh = dynamic_cast<Scene::CMeshInstance*>(entity))
        {
            auto faceVertNames =
                mesh->RemoveFace(faceNames); // silently do nothing if there is no face with those
                                             // corresponding facepoints
            std::cout << "tangible " + std::to_string(faceVertNames.size()) << std::endl;
            if (faceVertNames.size() != 0)
            {
                removedfaceNames.push_back(mesh->GetSceneTreeNode()->GetPath());
            }

            // DEBUG WHY THE FACEVERTNAMES ARE DIFFERENT THAN THE FACENAMES . it's because BSline
            // and sweep points aren't added into entitylibrary
            // if (faceVertNames.size() > 0) // if removed a face
            //{
            //    for (auto debug : faceVertNames)
            //        std::cout << "herey " + debug << std::endl;
            //    AddFace(faceVertNames);
            //}
            std::cout << "inside temp mesh maanger, finished call to remove face." << std::endl;
            // TODO: also need to remove corresponding interactivemesh. This can be done by removing
            // the node
        }
    });
}

// This function should roughly mirror the structure of CASTSceneBuilder::VisitFace
void CTemporaryMeshManager::AddFace(const std::vector<std::string>& facePoints)
{
    // if any of the facePoints contain a Temp name,
    bool createNewMesh = false;
    for (auto facePoint : facePoints)
    {
        std::cout << "currfacePoint name: " + facePoint << std::endl;
        std::cout << addedMeshes.size() << std::endl;
        if (facePoint.find("TempMesh") != std::string::npos
            || addedMeshes.size() == 0) // if it contains a TempMesh or this is the first face being
                                        // added in the session
            createNewMesh = true;
    }
    if (createNewMesh)
    { // if at least one of our facepoints references a TempMesh vert, we need to make a new
      // TempMesh.
        std::cout << "create new mesh " << std::endl;
        const std::string meshName = "TempMesh" + std::to_string(FaceCounter);
        const std::string faceName = meshName + "nnew" + std::to_string(FaceCounter);
        TAutoPtr<CFace> face = new CFace(faceName);
        Scene->AddEntity(tc::static_pointer_cast<CEntity>(
            face)); // Add face as an entity. Technically, this step can be skipped because we are
                    // directly adding it to a single mesh below
        std::cout << "add face: added face into the scene!" << std::endl;
        face->SetPointSourceNames(Scene,
                                  facePoints); // this is the point before any transformations
        TAutoPtr<CMesh> dummyMesh = new CMesh(meshName);
        dummyMesh->Faces.Connect(face->Face);
        Scene->AddEntity(tc::static_pointer_cast<CEntity>(
            dummyMesh)); // needs to instanciated as a mesh face, not just a face
        std::cout << "add face: added mesh into the scene!" << std::endl;
        auto sceneNode = Scene->GetRootNode()->CreateChildNode("inst" + meshName);
        std::cout << "add face: created new node!" << std::endl;
        auto entity = Scene->FindEntity(meshName); // entity needs to be the dummyMesh, instead of
                                                   // the face as we can't instanciate a face rn
        sceneNode->SetEntity(entity); // this doesn't work right now because you can't create an
                                      // instance of a single face

        addedSceneNodes.push_back(sceneNode);
        addedMeshes.push_back(dummyMesh);
        FaceCounter += 1;
        std::cout << "at the bottom of add face, this means were are done adding the face I think"
                  << std::endl;
    }
    else
    {
        std::cout << "the facepoints do not contain a tempmesh point and is not the first mesh "
                     "being added"
                  << std::endl;
        auto dummyMesh = addedMeshes.back(); //  get the most recently added mesh
        auto sceneNode = addedSceneNodes.back(); // get the most recently added scene node
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

// Highlight selected edge by adding temp polyline
void CTemporaryMeshManager::SelectOrDeselectPolyline(const std::vector<std::string>& points)
{
    auto searchpoint0 = points[0].substr(0, points[0].find("._"));
    auto searchpoint1 = points[0].substr(0, points[0].find("._"));
    std::cout << searchpoint0 << std::endl;
    std::cout << searchpoint1 << std::endl;
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
        std::cout << "trying to deselect" << std::endl;
        // const std::string polyName = points[0] + points[1]; // name is just the edge vert names
        // concatenated CSceneNode *temp;
        Scene->ForEachSceneTreeNode([&](Scene::CSceneTreeNode* node) {
            if (node->GetOwner()->GetName() == removeName)
            { // removeName includes the "inst" as prefix
                std::cout << "found the old polyline, trying to remove " << std::endl;
                std::cout << removeName << std::endl;
                node->GetOwner()->SetEntity(nullptr);
            }
        });
    }
    else // Select
    {
        std::cout << "trying to select" << std::endl;
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
    return "";
}

}