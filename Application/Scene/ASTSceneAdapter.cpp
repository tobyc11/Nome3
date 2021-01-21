#include "ASTSceneAdapter.h"
#include "BSpline.h"
#include "BezierSpline.h"
#include "Circle.h"
#include "Environment.h"
#include "Face.h"
#include "Funnel.h"
#include "Point.h"
#include "Polyline.h"
#include "Surface.h"
#include "Sweep.h"
#include "SweepControlPoint.h"
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
    { "circle", ECommandKind::Entity },      { "funnel", ECommandKind::Entity },
    { "tunnel", ECommandKind::Entity },      { "beziercurve", ECommandKind::Entity },
    { "bspline", ECommandKind::Entity },     { "instance", ECommandKind::Instance },
    { "surface", ECommandKind::Entity },     { "background", ECommandKind::Dummy },
    { "foreground", ECommandKind::Dummy },   { "insidefaces", ECommandKind::Dummy },
    { "outsidefaces", ECommandKind::Dummy }, { "offsetfaces", ECommandKind::Dummy },
    { "frontfaces", ECommandKind::Dummy },   { "backfaces", ECommandKind::Dummy },
    { "rimfaces", ECommandKind::Dummy },     { "bank", ECommandKind::BankSet },
    { "set", ECommandKind::BankSet },        { "delete", ECommandKind::Instance },
    { "subdivision", ECommandKind::Dummy },  { "offset", ECommandKind::Dummy }
};

ECommandKind CASTSceneAdapter::ClassifyCommand(const std::string& cmd) { return CommandInfoMap.at(cmd); }

CEntity* CASTSceneAdapter::MakeEntity(const std::string& cmd, const std::string& name)
{
    if (cmd == "beziercurve")
        return new CBezierSpline(name);
    else if (cmd == "bspline")
        return new CBSpline(name);
    else if (cmd == "circle")
        return new CCircle(name);
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
    else if (cmd == "sweep")
        return new CSweep(name);
    else if (cmd == "sweepcontrol")
        return new CSweepControlPoint(name);
    else if (cmd == "surface")
        return new CSurface(name);
    else if (cmd == "tunnel")
        return new CTunnel(name);
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
            auto* expr = cmd->GetPositionalArgument(index)->GetExpr();
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

void CASTSceneAdapter::VisitCommandSyncScene(AST::ACommand* cmd, CScene& scene, bool insubMesh)
{
    CmdTraverseStack.push_back(cmd);
    auto kind = ClassifyCommand(cmd->GetCommand());
    printf("%s: %d\n", cmd->GetCommand().c_str(), static_cast<int>(kind));
    if (kind == ECommandKind::Dummy)
    {
        printf("Warning: %s command unrecognized.\n", cmd->GetCommand().c_str());
    }
    else if (kind == ECommandKind::Entity)
    {

        TAutoPtr<CEntity> entity = MakeEntity(cmd->GetCommand(), EntityNamePrefix + cmd->GetName());
        entity->GetMetaObject().DeserializeFromAST(*cmd, *entity);
        GEnv.Scene->AddEntity(entity);

        if (auto* mesh = dynamic_cast<CMesh*>(ParentEntity))
        {
            if (auto* face = dynamic_cast<CFace*>(entity.Get()))
            {
                mesh->Faces.Connect(face->Face);
            }
        }

        // Added insubMesh bool to allow Meshes to process multiple subcommands (more than one).
        // Previously, there was an error where the subcommands' ParentEntity were not always
        // pointing to the mesh.
        if (!insubMesh)
        {
            ParentEntity = entity;
            EntityNamePrefix = cmd->GetName() + ".";
        }

        for (auto* sub : cmd->GetSubCommands())
        {
            VisitCommandSyncScene(sub, scene, true);
        }

        // Added insubMesh bool to allow Meshes to process multiple subcommands (more than one).
        // Previously, there was an error where EntityNamePrefix and ParentEntity were being reset
        // before the subcommands were finished processing.
        if (!insubMesh)
        {
            EntityNamePrefix = "";
            ParentEntity = nullptr;
        }
    }
    else if (cmd->GetCommand() == "instance")
    {
        auto* sceneNode = InstanciateUnder->CreateChildNode(cmd->GetName());
        sceneNode->SyncFromAST(cmd, scene);
        // TODO: move the following logic into SyncFromAST
        auto entityName = cmd->GetPositionalArgument(1)->ToString();
        auto entity = GEnv.Scene->FindEntity(entityName);
        if (entity)
            sceneNode->SetEntity(entity);
        else if (auto group = GEnv.Scene->FindGroup(entityName))
            group->AddParent(sceneNode);
        else
            throw AST::CSemanticError(
                tc::StringPrintf("Instantiation failed, unknown generator: %s", entityName.c_str()), cmd);
    }
    else if (cmd->GetCommand() == "group")
    {
        InstanciateUnder = GEnv.Scene->CreateGroup(cmd->GetName());
        InstanciateUnder->SyncFromAST(cmd, scene);
        for (auto* sub : cmd->GetSubCommands())
        {
            VisitCommandSyncScene(sub, scene, false);
        }
        InstanciateUnder = GEnv.Scene->GetRootNode();
    }
    CmdTraverseStack.pop_back();
}

CTransform* CASTSceneAdapter::ConvertASTTransform(AST::ANamedArgument* namedArg)
{
    auto items = cast<AST::AVector>(namedArg->GetArgument(0))->GetItems();
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
        auto v2 = cast<AST::AVector>(namedArg->GetArgument(1))->GetItems();
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