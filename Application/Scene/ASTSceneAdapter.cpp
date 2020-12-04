#include "ASTSceneAdapter.h"
#include "BSpline.h"
#include "BezierSpline.h"
#include "Circle.h"
#include "Cylinder.h"
#include "Environment.h"
#include "Face.h"
#include "Funnel.h"
#include "Helix.h"
#include "MeshMerger.h"
#include "MobiusStrip.h"
#include "Point.h"
#include "Polyline.h"
#include "Sphere.h"
#include "Surface.h"
#include "Sweep.h"
#include "SweepControlPoint.h"
#include "Torus.h"
#include "TorusKnot.h"
#include "Tunnel.h"
#include <StringPrintf.h>
#include <unordered_map>

namespace Nome::Scene
{

/*
 * Some notes for when I was looking through Nom.g4 trying to generalize things as much as possible.
 * The only primitive binding types I could find:
 * ident <-> std::string name
 * expression <-> float
 * ident <-> scene object reference
 * sub-command <-> ???
 *
 * Aside from that ident and expressions can also appear inside vectors and have variable length
 * bindings.
 */

static const std::unordered_map<std::string, ECommandKind> CommandInfoMap = {
    { "point", ECommandKind::Entity },       { "polyline", ECommandKind::Entity },
    { "sweep", ECommandKind::Entity },       { "sweepcontrol", ECommandKind::Entity },
    { "face", ECommandKind::Entity },        { "object", ECommandKind::Entity },
    { "mesh", ECommandKind::Entity },        { "group", ECommandKind::Instance },
    { "circle", ECommandKind::Entity },      { "sphere", ECommandKind::Entity },
    { "cylinder", ECommandKind::Entity },    { "funnel", ECommandKind::Entity },
    { "tunnel", ECommandKind::Entity },      { "beziercurve", ECommandKind::Entity },
    { "torusknot", ECommandKind::Entity },   { "torus", ECommandKind::Entity },
    { "bspline", ECommandKind::Entity },     { "instance", ECommandKind::Instance },
    { "surface", ECommandKind::Entity },     { "background", ECommandKind::Dummy },
    { "foreground", ECommandKind::Dummy },   { "insidefaces", ECommandKind::Dummy },
    { "outsidefaces", ECommandKind::Dummy }, { "offsetfaces", ECommandKind::Dummy },
    { "frontfaces", ECommandKind::Dummy },   { "backfaces", ECommandKind::Dummy },
    { "rimfaces", ECommandKind::Dummy },     { "bank", ECommandKind::BankSet },
    { "set", ECommandKind::BankSet },        { "delete", ECommandKind::Instance },
    { "subdivision", ECommandKind::Instance },  { "offset", ECommandKind::Dummy },
    { "mobiusstrip", ECommandKind::Entity }, {"helix", ECommandKind::Entity },
    {"sharp", ECommandKind::Entity }
};

ECommandKind CASTSceneAdapter::ClassifyCommand(const std::string& cmd)
{
    return CommandInfoMap.at(cmd);
}

CEntity* CASTSceneAdapter::MakeEntity(const std::string& cmd, const std::string& name)
{
    if (cmd == "beziercurve")
        return new CBezierSpline(name);
    else if (cmd == "bspline")
        return new CBSpline(name);
    else if (cmd == "circle")
        return new CCircle(name);
    else if (cmd == "cylinder")
        return new CCylinder(name);
    else if (cmd == "face")
        return new CFace(name);
    else if (cmd == "funnel")
        return new CFunnel(name);
    else if (cmd == "mesh" || cmd == "object")
        return new CMesh(name);
    else if (cmd == "point")
        return new CPoint(name);
    else if (cmd == "polyline")
        return new CPolyline(name);
    else if (cmd == "helix")
        return new CHelix(name);
    else if (cmd == "sphere")
        return new CSphere(name);
    else if (cmd == "sweep")
        return new CSweep(name);
    else if (cmd == "sweepcontrol")
        return new CSweepControlPoint(name);
    else if (cmd == "surface")
        return new CSurface(name);
    else if (cmd == "tunnel")
        return new CTunnel(name);
    else if (cmd == "torusknot")
        return new CTorusKnot(name);
    else if (cmd == "torus")
        return new CTorus(name);
    else if (cmd == "mobiusstrip")
        return new CMobiusStrip(name);

    return nullptr;
}

void CASTSceneAdapter::TraverseFile(AST::AFile* astRoot, CScene& scene)
{
    assert(CmdTraverseStack.empty());
    for (auto* cmd : astRoot->GetCommands())
        VisitCommandBankSet(cmd, scene);
    InstanciateUnder = GEnv.Scene->GetRootNode();
    for (auto* cmd : astRoot->GetCommands())
        VisitCommandSyncScene(cmd, scene, false);
}

void CASTSceneAdapter::VisitCommandBankSet(AST::ACommand* cmd, CScene& scene)
{
    CmdTraverseStack.push_back(cmd);
    auto kind = ClassifyCommand(cmd->GetCommand());
    if (kind == ECommandKind::BankSet && cmd->GetCommand() == "set")
    {
        if (CmdTraverseStack.rbegin()[1]->GetCommand() != "bank")
            throw AST::CSemanticError("Set command is not under bank", cmd);
        auto bank = CmdTraverseStack.rbegin()[1]->GetName();
        // You could generalize this somehow
        auto evalArg = [cmd](size_t index) {
            auto* expr = cmd->GetPositionalArgument(index);
            CExprEvalDirect eval;
            auto result = expr->Accept(&eval);
            return std::any_cast<float>(result);
        };
        auto name = bank + "." + cmd->GetName();
        scene.GetBankAndSet().AddSlider(name, cmd, evalArg(1), evalArg(2), evalArg(3), evalArg(4));
    }
    
    for (auto* sub : cmd->GetSubCommands())
        VisitCommandBankSet(sub, scene);
    CmdTraverseStack.pop_back();
}

void CASTSceneAdapter::IterateSharpness(AST::ACommand* cmd, CScene& scene) {

}


void CASTSceneAdapter::VisitCommandSyncScene(AST::ACommand* cmd, CScene& scene, bool insubMesh)
{
    CmdTraverseStack.push_back(cmd);
    auto kind = ClassifyCommand(cmd->GetCommand());
    printf("%s: %d\n", cmd->GetCommand().c_str(), kind);
    if (kind == ECommandKind::Dummy)
    {
        printf("Warning: %s command unrecognized.\n", cmd->GetCommand().c_str());
    }
    else if (kind == ECommandKind::Entity)
    {
        if (cmd->GetCommand() == "Sharp") {
            for (auto* sub : cmd->GetSubCommands())
            {
                sub->PushPositionalArgument(cmd->GetLevel());
                IterateSharpness(sub, scene);
            }


        } else
        {
            TAutoPtr<CEntity> entity =
                MakeEntity(cmd->GetCommand(), EntityNamePrefix + cmd->GetName());
            entity->GetMetaObject().DeserializeFromAST(*cmd, *entity);
            // All entities are added to the EntityLibrary dictionary
            GEnv.Scene->AddEntity(entity);
            if (auto* mesh = dynamic_cast<CMesh*>(ParentEntity))
                if (auto* face = dynamic_cast<CFace*>(entity.Get()))
                    mesh->Faces.Connect(face->Face);

            // Added insubMesh bool to allow Meshes to process multiple subcommands (more than one face).
            if (insubMesh == false)
            {
                ParentEntity = entity;
                EntityNamePrefix = cmd->GetName() + ".";
            }

            for (auto* sub : cmd->GetSubCommands())
                VisitCommandSyncScene(sub, scene, true);

            // Added insubMesh bool to allow Meshes to process multiple faces.
            if (insubMesh == false)
            {
                EntityNamePrefix = "";
                ParentEntity = nullptr;
            }
        }
    }
    else if (cmd->GetCommand() == "instance")
    {
        // Instance transformations/surfaces are not handled in here,
        auto* sceneNode = InstanciateUnder->CreateChildNode(cmd->GetName());
        // To perform rotation
        sceneNode->SyncFromAST(cmd, scene);
        // TODO: move the following logic into SyncFromAST

        // Check to see if there is a surface color associated with this instance. If the surface argument exists, then set the scene node's surface to be it.
        // Surface color for group vs mesh instance logic is handled in InteractiveMesh.cpp (at the rendering stage).
        auto surface = cmd->GetNamedArgument("surface");
        if (surface)
        {
            auto surfaceEntityNameExpr = surface->GetArgument(
                0)[0]; // Returns a casted AExpr that was an AIdent before casting
            auto surfaceIdentifier = static_cast<AST::AIdent*>(&surfaceEntityNameExpr)->ToString(); // Downcast it back to an AIdent
            auto surfaceEntity = GEnv.Scene->FindEntity(surfaceIdentifier);
            if (surfaceEntity)
                sceneNode->SetSurface(dynamic_cast<CSurface*>(surfaceEntity.Get()));

        }

        auto entityName = cmd->GetPositionalIdentAsString(1);
        auto entity = GEnv.Scene->FindEntity(entityName);
        if (entity)
            sceneNode->SetEntity(entity);
        else if (auto group = GEnv.Scene->FindGroup(entityName)) // If the entityName is a group identifier
            group->AddParent(sceneNode);
        else
            throw AST::CSemanticError(
                tc::StringPrintf("Instantiation failed, unknown generator: %s", entityName.c_str()),
                cmd);
    }
    else if (cmd->GetCommand() == "group")
    {
        InstanciateUnder = GEnv.Scene->CreateGroup(cmd->GetName());
        InstanciateUnder->SyncFromAST(cmd, scene);
        for (auto* sub : cmd->GetSubCommands())
            VisitCommandSyncScene(sub, scene, false);
        InstanciateUnder = GEnv.Scene->GetRootNode();
    }
    else if (cmd->GetCommand() == "subdivision") {
        // 1.read all instances to a merged mesh
        InstanciateUnder = GEnv.Scene->CreateMerge(cmd->GetName());
        InstanciateUnder->SyncFromAST(cmd, scene);
        //cmd->GetLevel();

        for (auto* sub : cmd->GetSubCommands())
            VisitCommandSyncScene(sub, scene, false);
        // auto node = InstanciateUnder;
        InstanciateUnder->AddParent(GEnv.Scene->GetRootNode());
        InstanciateUnder = GEnv.Scene->GetRootNode();
        // 2. merge all instances
        tc::TAutoPtr<Scene::CMeshMerger> merger = new Scene::CMeshMerger(cmd->GetName());
        merger->GetMetaObject().DeserializeFromAST(*cmd, *merger);

        /*
        node->ForEachTreeNode([&](Scene::CSceneTreeNode* node) {
            auto* entity = node->GetInstanceEntity(); // Else, get the instance
            if (!entity) // Check to see if the an entity is instantiable (e.g., polyline, funnel, mesh, etc.), and not just an instance identifier.
                entity = node->GetOwner()->GetEntity(); // If it's not instantiable, get entity instead of instance entity

            if (auto* mesh = dynamic_cast<Scene::CMeshInstance*>(entity))  //set "auto * mesh" to this entity. Call MergeIn to set merger's vertices based on mesh's vertices. Reminder: an instance identifier is NOT a Mesh, so only real entities get merged.
                merger->MergeIn(*mesh);
        });
         */

        scene.AddEntity(tc::static_pointer_cast<Scene::CEntity>(merger)); // Merger now has all the vertices set, so we can add it into the scene as a new entity
        auto* sn = scene.GetRootNode()->FindOrCreateChildNode(cmd->GetName()); //Add it into the Scene Tree by creating a new node called globalMergeNode. Notice, this is the same name everytime you Merge. This means you can only have one merger mesh each time. It will override previous merger meshes with the new vertices.
        sn->SetEntity(merger.Get()); // Set sn, which is the scene node, to point to entity merger
        //scene.Update();

        //tc::TAutoPtr<Scene::CMeshMerger> subdivision = new Scene::CMeshMerger(cmd->GetName()); //CmeshMerger is basically a CMesh, but with a MergeIn method. Merger will contain ALL the merged vertices (from various meshes)


        //scene.Update();
        /*
        node->ForEachTreeNode([&](Scene::CSceneTreeNode* node) {
            if (node->GetOwner()->GetName() == cmd->GetName())
            {
                auto* entity = node->GetInstanceEntity(); // this is non-null if the entity is
                // instantiable like a torus knot or polyline
                if (!entity) // if it's not instantiable, like a face, then get the entity associated
                    // with it
                    entity = node->GetOwner()->GetEntity();
                if (auto* mesh = dynamic_cast<Scene::CMeshInstance*>(entity))
                {
                    subdivision->Catmull(*mesh);
                }
            }
        });

        scene.AddEntity(tc::static_pointer_cast<Scene::CEntity>(subdivision)); // Merger now has all the vertices set, so we can add it into the scene as a new entity
        //sn = scene.GetRootNode()->FindOrCreateChildNode(cmd->GetName()); //Add it into the Scene Tree by creating a new node called globalMergeNode. Notice, this is the same name everytime you Merge. This means you can only have one merger mesh each time. It will override previous merger meshes with the new vertices.
        //sn->SetEntity(subdivision.Get());


        // TODO:Instantiate the subdivision node
        //auto subdivision = new Scene::CSubdivision(cmd->GetCommand(), cmd->GetLevel());
        //treeNode->InstanceEntity = Entity->Instantiate(treeNode);
        */

    }
    CmdTraverseStack.pop_back();
}

CTransform* CASTSceneAdapter::ConvertASTTransform(AST::ANamedArgument* namedArg)
{
    auto items = static_cast<AST::AVector*>(namedArg->GetArgument(0))->GetItems();
    if (namedArg->GetName() == "translate")
    {
        auto* transform = new CTranslate();
        CExprToNodeGraph c1 { items.at(0), GEnv.Scene->GetBankAndSet(), &transform->X };
        CExprToNodeGraph c2 { items.at(1), GEnv.Scene->GetBankAndSet(), &transform->Y };
        CExprToNodeGraph c3 { items.at(2), GEnv.Scene->GetBankAndSet(), &transform->Z };
        return transform;
    }
    else if (namedArg->GetName() == "rotate")
    {
        auto* transform = new CRotate();
        CExprToNodeGraph c1 { items.at(0), GEnv.Scene->GetBankAndSet(), &transform->AxisX };
        CExprToNodeGraph c2 { items.at(1), GEnv.Scene->GetBankAndSet(), &transform->AxisY };
        CExprToNodeGraph c3 { items.at(2), GEnv.Scene->GetBankAndSet(), &transform->AxisZ };
        auto v2 = static_cast<AST::AVector*>(namedArg->GetArgument(1))->GetItems();
        CExprToNodeGraph c4 { v2.at(0), GEnv.Scene->GetBankAndSet(), &transform->Angle };
        return transform;
    }
    else if (namedArg->GetName() == "scale")
    {
        auto* transform = new CScale();
        CExprToNodeGraph c1 { items.at(0), GEnv.Scene->GetBankAndSet(), &transform->X };
        CExprToNodeGraph c2 { items.at(1), GEnv.Scene->GetBankAndSet(), &transform->Y };
        CExprToNodeGraph c3 { items.at(2), GEnv.Scene->GetBankAndSet(), &transform->Z };
        return transform;
    }
    return nullptr;
}

}
