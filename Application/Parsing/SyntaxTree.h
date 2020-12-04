#pragma once
#include "StringBuffer.h"
#include <any>
#include <exception>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace Nome::AST
{

struct CBufLoc
{
    unsigned int BufId;
    unsigned int Start;
};

class CToken
{
public:
    CToken(std::string text, unsigned int bufId, unsigned int start);
    [[nodiscard]] std::string ToString() const;
    [[nodiscard]] const CBufLoc& GetLocation() const { return BufLoc; }
    void SetLocation(unsigned int bufId, unsigned int offset)
    {
        BufLoc.BufId = bufId;
        BufLoc.Start = offset;
    }
    void SetLocInvalid() { BufLoc.BufId = -1; }
    [[nodiscard]] bool IsLocInvalid() const { return BufLoc.BufId == -1; }

private:
    std::string Text;
    CBufLoc BufLoc;
};

enum class EKind : uint16_t
{
    Node,
    File,
    NamedArgument,
    NamedArgumentPost,
    Command,
    CommandPost,
    Expr,
    Ident,
    Number,
    UnaryOp,
    BinaryOp,
    Vector,
    Call,
    WrappedExpr,
    ExprPost,
    NodePost,
    LastKind
};

class ANode
{
public:
    bool CanBeChild(ANode* node) const;
    void AddChild(ANode* node);
    bool AddChildAfter(const ANode* after, ANode* child);
    [[nodiscard]] EKind GetKind() const { return Kind; }
    [[nodiscard]] CToken* GetOpenToken() const { return Token; }
    [[nodiscard]] CToken* GetCloseToken() const { return CloseToken; }

protected:
    ANode(EKind kind, EKind childKindBegin, EKind childKindEnd, CToken* token,
          CToken* closeToken = nullptr)
        : Kind(kind)
        , Token(token)
        , CloseToken(closeToken)
    {
        ChildKindRange =
            static_cast<uint32_t>(childKindBegin) | (static_cast<uint32_t>(childKindEnd) << 16);
    }

    EKind Kind;
    CToken* Token;
    CToken* CloseToken; // Optionally provide handling for matched open/close pairs
    std::vector<ANode*> Children;
    uint32_t ChildKindRange = 0;
};

//============================== Expressions ==============================

class IExprVisitor;

class AExpr : public ANode
{
public:
    std::any Accept(IExprVisitor* visitor);

    void CollectTokens(std::vector<CToken*>& tokenList) const;
    friend std::ostream& operator<<(std::ostream& os, const AExpr& node);

protected:
    AExpr(EKind kind, CToken* token, CToken* closeToken = nullptr)
        : ANode(kind, EKind::Expr, EKind::ExprPost, token, closeToken)
    {
    }
};

class AIdent : public AExpr
{
public:
    AIdent(CToken* token)
        : AExpr(EKind::Ident, token)
    {
    }

    std::string ToString() const { return Token->ToString(); }

    void CollectTokens(std::vector<CToken*>& tokenList) const { tokenList.push_back(Token); }
    friend std::ostream& operator<<(std::ostream& os, const AIdent& node)
    {
        os << node.ToString();
        return os;
    }
};

class ANumber : public AExpr
{
public:
    ANumber(CToken* token)
        : AExpr(EKind::Number, token)
    {
    }

    float AsFloat() const;
    double AsDouble() const;

    void CollectTokens(std::vector<CToken*>& tokenList) const { tokenList.push_back(Token); }
    friend std::ostream& operator<<(std::ostream& os, const ANumber& node)
    {
        os << node.AsDouble();
        return os;
    }
};

class AUnaryOp : public AExpr
{
public:
    enum class EOperator
    {
        Neg,
        Plus
    };

    AUnaryOp(CToken* token, AExpr* operand)
        : AExpr(EKind::UnaryOp, token)
    {
        AddChild(operand);
    }

    EOperator GetOperatorType() const;
    AExpr* GetOperand() const;

    void CollectTokens(std::vector<CToken*>& tokenList) const
    {
        tokenList.push_back(Token);
        GetOperand()->CollectTokens(tokenList);
    }
    friend std::ostream& operator<<(std::ostream& os, const AUnaryOp& node)
    {
        os << node.Token->ToString();
        os << *node.GetOperand();
        return os;
    }
};

class ABinaryOp : public AExpr
{
public:
    enum class EOperator
    {
        Add,
        Sub,
        Mul,
        Div,
        Exp
    };

    ABinaryOp(CToken* token, AExpr* left, AExpr* right)
        : AExpr(EKind::BinaryOp, token)
    {
        AddChild(left);
        AddChild(right);
    }

    EOperator GetOperatorType() const;
    AExpr* GetLeft() const;
    AExpr* GetRight() const;

    void CollectTokens(std::vector<CToken*>& tokenList) const
    {
        GetLeft()->CollectTokens(tokenList);
        tokenList.push_back(Token);
        GetRight()->CollectTokens(tokenList);
    }
    friend std::ostream& operator<<(std::ostream& os, const ABinaryOp& node)
    {
        os << *node.GetLeft();
        os << node.Token->ToString();
        os << *node.GetRight();
        return os;
    }
};

// Vector means a list in this context
class AVector : public AExpr
{
public:
    AVector(CToken* openToken, CToken* closeToken)
        : AExpr(EKind::Vector, openToken, closeToken)
    {
    }

    std::vector<AExpr*> GetItems() const;

    void CollectTokens(std::vector<CToken*>& tokenList) const
    {
        tokenList.push_back(Token);
        for (auto* expr : GetItems())
            expr->CollectTokens(tokenList);
        tokenList.push_back(CloseToken);
    }
    friend std::ostream& operator<<(std::ostream& os, const AVector& node)
    {
        os << "(";
        for (AExpr* expr : node.GetItems())
            os << *expr << " ";
        os << ")";
        return os;
    }
};

class ACall : public AExpr
{
public:
    ACall(CToken* funcToken, AVector* argumentList);

    std::string GetFuncName() const { return Token->ToString(); }
    AVector* GetOperandList() const { return static_cast<AVector*>(Children[0]); }

    void CollectTokens(std::vector<CToken*>& tokenList) const
    {
        tokenList.push_back(Token);
        GetOperandList()->CollectTokens(tokenList);
    }
    friend std::ostream& operator<<(std::ostream& os, const ACall& node)
    {
        os << node.GetFuncName();
        os << *node.GetOperandList();
        return os;
    }
};

class AWrappedExpr : public AExpr
{
public:
    AWrappedExpr(CToken* beginToken, CToken* endToken, CToken* secondToken, AExpr* expr)
        : AExpr(EKind::WrappedExpr, beginToken, endToken)
        , SecondToken(secondToken)
    {
        AddChild(expr);
    }

    AExpr* GetExpr() const { return static_cast<AExpr*>(Children[0]); }

    void CollectTokens(std::vector<CToken*>& tokenList) const
    {
        tokenList.push_back(Token);
        if (SecondToken)
            tokenList.push_back(SecondToken);
        GetExpr()->CollectTokens(tokenList);
        tokenList.push_back(CloseToken);
    }
    friend std::ostream& operator<<(std::ostream& os, const AWrappedExpr& node)
    {
        os << *node.GetExpr();
        return os;
    }

private:
    CToken* SecondToken;
};

class IExprVisitor
{
public:
    virtual ~IExprVisitor();

    virtual std::any VisitIdent(AIdent* ident) = 0;
    virtual std::any VisitNumber(ANumber* number) = 0;
    virtual std::any VisitUnaryOp(AUnaryOp* unaryOp) = 0;
    virtual std::any VisitBinaryOp(ABinaryOp* binaryOp) = 0;
    virtual std::any VisitCall(ACall* call) = 0;
    virtual std::any VisitVector(AVector* vector) = 0;
    virtual std::any VisitWrappedExpr(AWrappedExpr* vector) = 0;
};

//============================== File ==============================

class ACommand;

class AFile : public ANode
{
public:
    AFile()
        : ANode(EKind::File, EKind::Command, EKind::CommandPost, nullptr)
    {
    }

    std::vector<ACommand*> GetCommands() const;
    void CollectTokens(std::vector<CToken*>& tokenList) const;

    friend std::ostream& operator<<(std::ostream& os, const AFile& node);
};

class ANamedArgument : public ANode
{
public:
    ANamedArgument(CToken* nameToken)
        : ANode(EKind::NamedArgument, EKind::Expr, EKind::ExprPost, nameToken)
    {
    }

    std::string GetName() const;
    AExpr* GetArgument(size_t index) const;
    std::vector<AExpr*> GetArguments() const;
    void CollectTokens(std::vector<CToken*>& tokenList) const;

    friend std::ostream& operator<<(std::ostream& os, const ANamedArgument& node);
};

class ACommand : public ANode
{
public:
    ACommand(CToken* openToken, CToken* closeToken);

    // `Children` field of ACommand only stores sub-commands
    void AddSubCommand(ACommand* subCommand) { AddChild(subCommand); }
    // The arguments are stored separately as private fields
    //     Named arguments and transforms are both ANamedArgument, but transforms are ordered
    void PushPositionalArgument(AExpr* expr) { PositionalArguments.push_back(expr); }
    void SetPositionalArgument(size_t i, AExpr* expr) { PositionalArguments[i] = expr; }
    void AddNamedArgument(ANamedArgument* argument);
    void AddTransform(ANamedArgument* subCommand) { Transforms.push_back(subCommand); }


    std::string GetCommand() const { return Token->ToString(); }
    std::string GetName() const { return GetPositionalIdentAsString(0); }
    int GetLevel() const { return GetPositionalNumber(1); }
    std::string GetPositionalIdentAsString(size_t index) const;
    int GetPositionalNumber(size_t index) const;
    AExpr* GetPositionalArgument(size_t index) const;
    ANamedArgument* GetNamedArgument(const std::string& name) const;
    const std::vector<ANamedArgument*>& GetTransforms() const { return Transforms; }

    std::vector<ACommand*> GetSubCommands() const;
    void CollectTokens(std::vector<CToken*>& tokenList) const;

    friend std::ostream& operator<<(std::ostream& os, const ACommand& node);

    bool IsPendingSave() const { return bPendingSave; }
    void SetPendingSave(bool value) { bPendingSave = value; }

private:
    std::vector<AExpr*> PositionalArguments;
    std::map<std::string, ANamedArgument*> NamedArguments;
    std::vector<ANamedArgument*> Transforms;
    bool bPendingSave = false;
};

//============================== Misc ==============================

class CSemanticError : public std::exception
{
public:
    CSemanticError(std::string message, const ANode* where)
        : Message(std::move(message))
        , Node(where)
    {
    }

    [[nodiscard]] char const* what() const noexcept override;
    [[nodiscard]] const ANode* GetNode() const { return Node; }

private:
    std::string Message;
    const ANode* Node;
};
}
