#include "AST.h"

namespace Nome
{

AExpr* AExpr::Create(CASTContext& ctx, CSourceLocation beginLoc, CSourceLocation endLoc)
{
    void* mem = reinterpret_cast<AExpr*>(ctx.Alloc(alignof(AExpr), sizeof(AExpr)));
    auto* result = new (mem) AExpr(beginLoc, endLoc);
    return result;
}

AExpr::AExpr() : ClassId(EClassId::Expr)
{
}

AExpr::AExpr(CSourceLocation beginLoc, CSourceLocation endLoc) : ClassId(EClassId::Expr), BeginLoc(beginLoc), EndLoc(endLoc)
{
}

AKeyword::AKeyword(const std::string& keyword, CSourceLocation beginLoc, CSourceLocation endLoc)
    : AExpr(beginLoc, endLoc), Keyword(keyword)
{
    ClassId = EClassId::Keyword;
}

AKeyword* AKeyword::Create(CASTContext& ctx, const std::string& kwd, CSourceLocation beginLoc, CSourceLocation endLoc)
{
	void* mem = reinterpret_cast<AKeyword*>(ctx.Alloc(alignof(AKeyword), sizeof(AKeyword)));
	auto* result = new (mem) AKeyword(kwd, beginLoc, endLoc);
	return result;
}

AIdent::AIdent(const std::string& ident, CSourceLocation beginLoc, CSourceLocation endLoc)
    : AExpr(beginLoc, endLoc), Identifier(ident)
{
    ClassId = EClassId::Ident;
}

AIdent* AIdent::Create(CASTContext& ctx, const std::string& id, CSourceLocation beginLoc, CSourceLocation endLoc)
{
	void* mem = reinterpret_cast<AIdent*>(ctx.Alloc(alignof(AIdent), sizeof(AIdent)));
	auto* result = new (mem) AIdent(id, beginLoc, endLoc);
	return result;
}

ANumber::ANumber(const std::string& stringVal, CSourceLocation beginLoc, CSourceLocation endLoc)
    : AExpr(beginLoc, endLoc), String(stringVal)
{
    ClassId = EClassId::Number;
    Value = std::stod(String);
}

ANumber* ANumber::Create(CASTContext& ctx, const std::string& stringVal, CSourceLocation beginLoc, CSourceLocation endLoc)
{
	void* mem = ctx.Alloc(alignof(ANumber), sizeof(ANumber));
	auto result = new (mem) ANumber(stringVal, beginLoc, endLoc);
    return result;
}

AUnaryOp* AUnaryOp::Create(CASTContext& ctx, AUnaryOp::EOperator type, AExpr* operand, AKeyword* operatorToken)
{
    void* mem = ctx.Alloc(alignof(AUnaryOp), sizeof(AUnaryOp));
    auto result = new (mem) AUnaryOp(type, operand, operatorToken);
    return result;
}

AUnaryOp::AUnaryOp(AUnaryOp::EOperator type, AExpr* operand, AKeyword* operatorToken)
    : Type(type), Operand(operand)
{
    ClassId = EClassId::UnaryOp;
}

ABinaryOp* ABinaryOp::Create(CASTContext& ctx, ABinaryOp::EOperator type, AExpr* left, AExpr* right, AKeyword* operatorToken)
{
    void* mem = ctx.Alloc(alignof(ABinaryOp), sizeof(ABinaryOp));
    auto result = new (mem) ABinaryOp(type, left, right, operatorToken);
    return result;
}

ABinaryOp::ABinaryOp(ABinaryOp::EOperator type, AExpr* left, AExpr* right, AKeyword* operatorToken)
    : Type(type), Left(left), Right(right)
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

ACommand* ACommand::Create(CASTContext& ctx, AIdent* name, AKeyword* beginKeyword, AKeyword* endKeyword)
{
    void* mem = ctx.Alloc(alignof(ACommand), sizeof(ACommand));
    auto result = new (mem) ACommand();
    result->Name = name;
    result->BeginKeyword = beginKeyword;
    result->EndKeyword = endKeyword;
    return result;
}

AExpr* ACommand::FindNamedArg(const std::string& name) const
{
	for (const auto& pair : NamedArgs)
	{
		if (pair.first->Keyword == name)
			return pair.second;
	}
	return nullptr;
}

}
