#pragma once
#include "ASTContext.h"
#include "SourceManager.h"
#include <vector>
#include <map>
#include <type_traits>

namespace Nome
{

enum class EClassId
{
    Expr,
    Keyword,
    Ident,
    Number,
    UnaryOp,
    BinaryOp,
    Transform,
    Command
};

class AExpr
{
public:
	inline static EClassId StaticId() { return EClassId::Expr; }
    static AExpr* Create(CASTContext& ctx, CSourceLocation beginLoc, CSourceLocation endLoc);
    EClassId ClassId;
	CSourceLocation BeginLoc;
	CSourceLocation EndLoc;

protected:
    AExpr();
    AExpr(CSourceLocation beginLoc, CSourceLocation endLoc);
};

//Not really an expression. Inherits AExpr as a convenience.
class AKeyword : public AExpr
{
	AKeyword(const std::string& keyword, CSourceLocation beginLoc, CSourceLocation endLoc);

public:
	inline static EClassId StaticId() { return EClassId::Keyword; }
	static AKeyword* Create(CASTContext& ctx, const std::string& kwd, CSourceLocation beginLoc, CSourceLocation endLoc);

	std::string Keyword;
};

class AIdent : public AExpr
{
	AIdent(const std::string& ident, CSourceLocation beginLoc, CSourceLocation endLoc);

public:
	inline static EClassId StaticId() { return EClassId::Ident; }
	static AIdent* Create(CASTContext& ctx, const std::string& id, CSourceLocation beginLoc, CSourceLocation endLoc);

	std::string Identifier;
};

class ANumber : public AExpr
{
	ANumber(const std::string& stringVal, CSourceLocation beginLoc, CSourceLocation endLoc);

public:
	inline static EClassId StaticId() { return EClassId::Number; }
	static ANumber* Create(CASTContext& ctx, const std::string& stringVal, CSourceLocation beginLoc, CSourceLocation endLoc);

	double GetValue() const { return Value; }

private:
	double Value;
	std::string String;
};

class AUnaryOp : public AExpr
{
public:
	enum EOperator
	{
		UOP_NEG,
		//Trig
		UOP_SIN,
		UOP_COS,
		UOP_TAN,
		UOP_COT,
		UOP_SEC,
		UOP_CSC,
		UOP_ARCSIN,
		UOP_ARCCOS,
		UOP_ARCTAN,
		UOP_ARCCOT,
		UOP_ARCSEC,
		UOP_ARCCSC,
	};

	EOperator Type;
	AExpr* Operand;

	inline static EClassId StaticId() { return EClassId::UnaryOp; }
	static AUnaryOp* Create(CASTContext& ctx, EOperator type, AExpr* operand, AKeyword* operatorToken);

private:
    AUnaryOp(EOperator type, AExpr* operand, AKeyword* operatorToken);
};

class ABinaryOp : public AExpr
{
public:
    enum EOperator
    {
        BOP_ADD,
        BOP_SUB,
        BOP_MUL,
        BOP_DIV,
        BOP_EXP
    };

	EOperator Type;
	AExpr* Left;
	AExpr* Right;

	inline static EClassId StaticId() { return EClassId::BinaryOp; }
	//No need for location, auto derive from left and right operands
	static ABinaryOp* Create(CASTContext& ctx, EOperator type, AExpr* left, AExpr* right, AKeyword* operatorToken = nullptr);

private:
    ABinaryOp(EOperator type, AExpr* left, AExpr* right, AKeyword* operatorToken);
};

//Not strictly an expr either
class ATransform : public AExpr
{
public:
    enum ETransformType
    {
        TF_ROTATE,
        TF_SCALE,
        TF_TRANSLATE
    } Type;

    AExpr* AxisX;
    AExpr* AxisY;
    AExpr* AxisZ;
    AExpr* Deg; //Only valid for rotation

	inline static EClassId StaticId() { return EClassId::Transform; }
    static ATransform* Create(CASTContext& ctx);
};

class ACommand
{
public:
    //Only BeginKeyword is required
    AKeyword* BeginKeyword;
    //Can be nullptr, e.g., `set`
    AKeyword* EndKeyword;
    //Can be nullptr if the command has no name like `delete` or `set`
    AIdent* Name;
    std::vector<AExpr*> Args;
    std::map<AKeyword*, AExpr*> NamedArgs;

    std::vector<ACommand*> SubCommands;

    EClassId ClassId = EClassId::Command;
	inline static EClassId StaticId() { return EClassId::Command; }
	static ACommand* Create(CASTContext& ctx, AIdent* name, AKeyword* beginKeyword, AKeyword* endKeyword);

	AExpr* FindNamedArg(const std::string& name) const;
};

template <typename TTo, typename TFrom, typename TToPlain = typename std::remove_pointer<TTo>::type>
TTo ast_as(TFrom* node)
{
	if (!node)
		return nullptr;
	if (node->ClassId == TToPlain::StaticId())
		return static_cast<TTo>(node);
	return nullptr;
}

}
