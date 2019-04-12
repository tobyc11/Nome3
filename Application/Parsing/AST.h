#pragma once
#include "ASTContext.h"
#include "SourceManager.h"
#include <map>
#include <type_traits>
#include <vector>

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
    ExprList,
    CommandArgument,
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

class AIdent : public AExpr
{
    AIdent(const std::string& ident, CSourceLocation beginLoc, CSourceLocation endLoc);

public:
    inline static EClassId StaticId() { return EClassId::Ident; }
    static AIdent* Create(CASTContext& ctx, const std::string& id, CSourceLocation beginLoc,
                          CSourceLocation endLoc);

    std::string Identifier;
};

class ANumber : public AExpr
{
    ANumber(const std::string& stringVal, CSourceLocation beginLoc, CSourceLocation endLoc);

public:
    inline static EClassId StaticId() { return EClassId::Number; }
    static ANumber* Create(CASTContext& ctx, const std::string& stringVal, CSourceLocation beginLoc,
                           CSourceLocation endLoc);

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
        // Trig
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
    static AUnaryOp* Create(CASTContext& ctx, EOperator type, AExpr* operand,
                            AIdent* operatorToken);

private:
    AUnaryOp(EOperator type, AExpr* operand, AIdent* operatorToken);
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
    // No need for location, auto derive from left and right operands
    static ABinaryOp* Create(CASTContext& ctx, EOperator type, AExpr* left, AExpr* right,
                             AIdent* operatorToken = nullptr);

private:
    ABinaryOp(EOperator type, AExpr* left, AExpr* right, AIdent* operatorToken);
};

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
    AExpr* Deg; // Only valid for rotation

    inline static EClassId StaticId() { return EClassId::Transform; }
    static ATransform* Create(CASTContext& ctx);
};

class AExprList : public AExpr
{
public:
    inline static EClassId StaticId() { return EClassId::ExprList; }

    static AExprList* Create(CASTContext& ctx);

    void AddExpr(AExpr* expr);
    const std::vector<AExpr*>& GetExpressions() const;
    std::vector<AIdent*> ConvertToIdents() const;

private:
    std::vector<AExpr*> Expressions;
};

// Corresponds to command_arg_list in the parser
class ACommandArgument
{
    friend class ACommand;

public:
    EClassId ClassId = EClassId::CommandArgument;
    inline static EClassId StaticId() { return EClassId::CommandArgument; }

    static ACommandArgument* Create(CASTContext& ctx, AIdent* param, AExpr* value,
                                    ACommandArgument* next = nullptr);

    ACommand* GetParent() const { return Parent; }
    ACommandArgument* GetPrev() const { return Prev; }
    ACommandArgument* GetNext() const { return Next; }
    AIdent* GetParameter() const { return Parameter; }
    AExpr* GetValue() const { return Value; }

private:
    ACommand* Parent = nullptr;
    ACommandArgument* Prev = nullptr;
    ACommandArgument* Next = nullptr;

    AIdent* Parameter;
    AExpr* Value;
};

class ACommand
{
public:
    EClassId ClassId = EClassId::Command;
    inline static EClassId StaticId() { return EClassId::Command; }
    static ACommand* Create(CASTContext& ctx, AIdent* name, AIdent* beginKeyword,
                            AIdent* endKeyword, ACommandArgument* args = nullptr,
                            ACommand* subcommands = nullptr);

    AIdent* GetBeginKeyword() const { return BeginKeyword; }
    AIdent* GetEndKeyword() const { return EndKeyword; }
    AIdent* GetName() const { return Name; }

    void SetNext(ACommand* next)
    {

        this->Next = next;
        if (next)
            next->Prev = this;
    }
    AExpr* FindNamedArg(const std::string& name) const;
    ACommandArgument* GetArguments() const { return Arguments; }
    void SetArguments(ACommandArgument* args)
    {
        Arguments = args;
        Arguments->Parent = this;
    }
    ACommandArgument* AddArgument(AIdent* param, AExpr* value);
    void AppendChild(ACommand* cmd);

    std::vector<ACommand*> GatherSubcommands() const;

private:
    ACommand(CASTContext& ctx)
        : Context(ctx)
    {
    }
    CASTContext& Context;

    ACommand* Parent = nullptr;
    ACommand* Prev = nullptr;
    ACommand* Next = nullptr;
    ACommand* FirstChild = nullptr; // Subcommands

    // Only BeginKeyword is required
    AIdent* BeginKeyword = nullptr;
    // Can be nullptr, e.g., `set`
    AIdent* EndKeyword = nullptr;
    // Can be nullptr if the command has no name like `delete` or `set`
    AIdent* Name = nullptr;
    ACommandArgument* Arguments = nullptr;
};

template <typename TTo, typename TFrom, typename TToPlain = typename std::remove_pointer<TTo>::type>
TTo ast_as(TFrom* node)
{
    if (!node)
        return nullptr;
    if (node->ClassId == TToPlain::StaticId())
        return static_cast<TTo>(node);
    // TODO: also consider up/down cast
    return nullptr;
}

class CSemanticError : public std::exception
{
public:
    CSemanticError(const char* message, AExpr* expr)
    {
        Message =
            std::string(message) + " " + expr->BeginLoc.ToString() + "-" + expr->EndLoc.ToString();
    }

    CSemanticError(const std::string& message, AExpr* expr)
    {
        Message = message + " " + expr->BeginLoc.ToString() + "-" + expr->EndLoc.ToString();
    }

    CSemanticError(const std::string& message, ACommand* cmd)
    {
        Message = message + " " + cmd->GetBeginKeyword()->BeginLoc.ToString();
    }

    char const* what() const override { return Message.c_str(); }

private:
    std::string Message;
};

}
