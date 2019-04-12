#include "NomeDriver.h"

namespace Nome
{

void ACommandExtHelper::MoveTo(CNomeDriver& driver, ACommand* dest)
{
    for (const auto& pair : NamedArgs)
    {
        auto* newArg = ACommandArgument::Create(*driver.GetASTContext(), pair.first, pair.second,
                                                dest->GetArguments());
        dest->SetArguments(newArg);
    }
    *this = ACommandExtHelper();
}

CNomeDriver::CNomeDriver(const std::string& sourceString)
    : SourceString(sourceString)
{
    Context = new CASTContext();
}

CNomeDriver::CNomeDriver(CASTContext* context, CSourceManager* sourceMgr, CSourceFile* sourceFile)
    : Context(context)
    , SourceMgr(sourceMgr)
    , SourceFile(sourceFile)
{
}

AIdent* CNomeDriver::MakeIdent(const std::string& identifier)
{
    return AIdent::Create(*Context, identifier, CSourceLocation(), CSourceLocation());
}

void CNomeDriver::MoveIdentList(ACommand* dest, const std::string& paramName)
{
    auto* list = AExprList::Create(*Context);
    for (AIdent* ident : IdentList)
        list->AddExpr(ident);
    dest->AddArgument(MakeIdent(paramName), list);
    IdentList.clear();
}

void CNomeDriver::MoveFaceList(ACommand* dest)
{
    for (ACommand* cmd : FaceList)
        dest->AppendChild(cmd);
    FaceList.clear();
}

void CNomeDriver::MoveInstanceList(ACommand* dest)
{
    for (ACommand* cmd : InstanceList)
        dest->AppendChild(cmd);
    InstanceList.clear();
}

void CNomeDriver::MoveSetList(ACommand* dest)
{
    for (ACommand* cmd : SetList)
        dest->AppendChild(cmd);
    SetList.clear();
}

void CNomeDriver::MoveFaceForDeletionList(ACommand* dest)
{
    for (ACommand* cmd : FaceForDeletionList)
        dest->AppendChild(cmd);
    FaceForDeletionList.clear();
}

void CNomeDriver::MoveTransformList(ACommand* dest, const std::string& paramName)
{
    auto* list = AExprList::Create(*Context);
    for (ATransform* transform : TransformList)
        list->AddExpr(transform);
    dest->AddArgument(MakeIdent(paramName), list);
    TransformList.clear();
}

}
