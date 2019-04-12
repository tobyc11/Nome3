#include "AST.h"
#include <algorithm>

namespace Nome
{

AExpr* AExpr::Create(CASTContext& ctx, CSourceLocation beginLoc, CSourceLocation endLoc)
{
    void* mem = reinterpret_cast<AExpr*>(ctx.Alloc(alignof(AExpr), sizeof(AExpr)));
    auto* result = new (mem) AExpr(beginLoc, endLoc);
    return result;
}

AExpr::AExpr()
    : ClassId(EClassId::Expr)
{
}

AExpr::AExpr(CSourceLocation beginLoc, CSourceLocation endLoc)
    : ClassId(EClassId::Expr)
    , BeginLoc(beginLoc)
    , EndLoc(endLoc)
{
}

AIdent::AIdent(const std::string& ident, CSourceLocation beginLoc, CSourceLocation endLoc)
    : AExpr(beginLoc, endLoc)
    , Identifier(ident)
{
    ClassId = EClassId::Ident;
}

AIdent* AIdent::Create(CASTContext& ctx, const std::string& id, CSourceLocation beginLoc,
                       CSourceLocation endLoc)
{
    void* mem = reinterpret_cast<AIdent*>(ctx.Alloc(alignof(AIdent), sizeof(AIdent)));
    auto* result = new (mem) AIdent(id, beginLoc, endLoc);
    return result;
}

ANumber::ANumber(const std::string& stringVal, CSourceLocation beginLoc, CSourceLocation endLoc)
    : AExpr(beginLoc, endLoc)
    , String(stringVal)
{
    ClassId = EClassId::Number;
    Value = std::stod(String);
}

ANumber* ANumber::Create(CASTContext& ctx, const std::string& stringVal, CSourceLocation beginLoc,
                         CSourceLocation endLoc)
{
    void* mem = ctx.Alloc(alignof(ANumber), sizeof(ANumber));
    auto result = new (mem) ANumber(stringVal, beginLoc, endLoc);
    return result;
}

AUnaryOp* AUnaryOp::Create(CASTContext& ctx, AUnaryOp::EOperator type, AExpr* operand,
                           AIdent* operatorToken)
{
    void* mem = ctx.Alloc(alignof(AUnaryOp), sizeof(AUnaryOp));
    auto result = new (mem) AUnaryOp(type, operand, operatorToken);
    return result;
}

AUnaryOp::AUnaryOp(AUnaryOp::EOperator type, AExpr* operand, AIdent* operatorToken)
    : Type(type)
    , Operand(operand)
{
    ClassId = EClassId::UnaryOp;
}

ABinaryOp* ABinaryOp::Create(CASTContext& ctx, ABinaryOp::EOperator type, AExpr* left, AExpr* right,
                             AIdent* operatorToken)
{
    void* mem = ctx.Alloc(alignof(ABinaryOp), sizeof(ABinaryOp));
    auto result = new (mem) ABinaryOp(type, left, right, operatorToken);
    return result;
}

ABinaryOp::ABinaryOp(ABinaryOp::EOperator type, AExpr* left, AExpr* right, AIdent* operatorToken)
    : Type(type)
    , Left(left)
    , Right(right)
{
    ClassId = EClassId::BinaryOp;
}

ATransform* ATransform::Create(CASTContext& ctx)
{
    void* mem = ctx.Alloc(alignof(ATransform), sizeof(ATransform));
    auto result = new (mem) ATransform();
    result->ClassId = EClassId::Transform;
    return result;
}

AExprList* AExprList::Create(CASTContext& ctx)
{
    void* mem = ctx.Alloc(alignof(AExprList), sizeof(AExprList));
    auto result = new (mem) AExprList();
    result->ClassId = EClassId::ExprList;
    return result;
}

void AExprList::AddExpr(AExpr* expr) { Expressions.push_back(expr); }

const std::vector<AExpr*>& AExprList::GetExpressions() const { return Expressions; }

std::vector<AIdent*> AExprList::ConvertToIdents() const
{
    std::vector<AIdent*> result;
    for (AExpr* expr : GetExpressions())
    {
        AIdent* ident = ast_as<AIdent*>(expr);
        if (!ident)
            throw CSemanticError("Cannot convert to ident", expr);
        else
            result.push_back(ident);
    }
    return result;
}

ACommandArgument* ACommandArgument::Create(CASTContext& ctx, AIdent* param, AExpr* value,
                                           ACommandArgument* next)
{
    void* mem = ctx.Alloc(alignof(ACommandArgument), sizeof(ACommandArgument));
    auto result = new (mem) ACommandArgument();
    result->Next = next;
    if (next)
        next->Prev = result;
    result->Parameter = param;
    result->Value = value;
    return result;
}

ACommand* ACommand::Create(CASTContext& ctx, AIdent* name, AIdent* beginKeyword, AIdent* endKeyword,
                           ACommandArgument* args, ACommand* subcommands)
{
    void* mem = ctx.Alloc(alignof(ACommand), sizeof(ACommand));
    auto result = new (mem) ACommand(ctx);
    result->Name = name;
    result->BeginKeyword = beginKeyword;
    result->EndKeyword = endKeyword;
    result->Arguments = args;
    if (args)
        args->Parent = result;
    result->FirstChild = subcommands;
    return result;
}

AExpr* ACommand::FindNamedArg(const std::string& name) const
{
    ACommandArgument* currArg = Arguments;
    while (currArg)
    {
        if (currArg->GetParameter()->Identifier == name)
            return currArg->GetValue();
        currArg = currArg->GetNext();
    }
    return nullptr;
}

ACommandArgument* ACommand::AddArgument(AIdent* param, AExpr* value)
{
    Arguments = ACommandArgument::Create(Context, param, value, Arguments);
    Arguments->Parent = this;
    return Arguments;
}

void ACommand::AppendChild(ACommand* cmd)
{
    cmd->Prev = nullptr;
    cmd->Next = FirstChild;
    cmd->Parent = this;
    if (FirstChild)
		FirstChild->Prev = cmd;
    FirstChild = cmd;
}

std::vector<ACommand*> ACommand::GatherSubcommands() const
{
    std::vector<ACommand*> cmds;
    ACommand* currCmd = FirstChild;
    while (currCmd)
    {
        cmds.push_back(currCmd);
        currCmd = currCmd->Next;
    }
    std::reverse(cmds.begin(), cmds.end());
    return cmds;
}

}
