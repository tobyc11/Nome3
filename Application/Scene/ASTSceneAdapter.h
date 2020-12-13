#pragma once
#include "Scene.h"
#include <Parsing/SyntaxTree.h>
#include <string>
#include <vector>

namespace Nome::Scene
{

enum class ECommandKind
{
    Dummy,
    Entity,
    Instance,
    BankSet,
    DocEdit
};

class CASTSceneAdapter
{
public:
    static ECommandKind ClassifyCommand(const std::string& cmd);
    static CEntity* MakeEntity(const std::string& cmd, const std::string& name);
    static CTransform* ConvertASTTransform(AST::ANamedArgument* namedArg);

    void TraverseFile(AST::AFile* astRoot, CScene& scene);
    std::vector<std::string> CASTSceneAdapter::GetIncludes(AST::AFile* astRoot, CScene& scene); // Randy added on 12/13

    void VisitCommandBankSet(AST::ACommand* cmd, CScene& scene);
    void VisitCommandSyncScene(AST::ACommand* cmd, CScene& scene, bool insubMesh);
    std::string VisitInclude(AST::ACommand* cmd, CScene& scene); // Randy added this on 11/30 for include files

    std::vector<AST::ACommand*> CmdTraverseStack;
    CSceneNode* InstanciateUnder = nullptr;
    std::string EntityNamePrefix;
    CEntity* ParentEntity = nullptr;
};

}