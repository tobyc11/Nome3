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
    BankSet
};

class CASTSceneAdapter
{
public:
    static ECommandKind ClassifyCommand(const std::string& cmd);
    static CEntity* MakeEntity(const std::string& cmd, const std::string& name);
    static CTransform* ConvertASTTransform(AST::ANamedArgument* namedArg);

    void TraverseFile(AST::AFile* astRoot, CScene& scene);

private:
    void VisitCommandBankSet(AST::ACommand* cmd, CScene& scene);
    void VisitCommandSyncScene(AST::ACommand* cmd, CScene& scene, bool insubMesh);

    std::vector<AST::ACommand*> CmdTraverseStack;
    CSceneNode* InstanciateUnder = nullptr;
    std::string EntityNamePrefix;
    CEntity* ParentEntity = nullptr;
    void IterateSharpness(AST::ACommand* cmd, CScene& scene);
};

}