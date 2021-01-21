#pragma once
#include "ASTContext.h"
#include "SourceManager.h"
#include <any>
#include <exception>
#include <map>
#include <string>
#include <vector>

/*
 * TODO:
 *   allow some flexibility by implementing the following
 *   SourceManager->InsertTextAndParse(location, string) => ANode
 *   ACommand::Create(location, cmdName, name)
 */

namespace Nome::AST
{

class CToken
{
protected:
    CToken(CSourceLocation beginLoc, unsigned len);

public:
    /// Creates a token object, here we assume that the text referred to exists
    static CToken* Create(CASTContext& ctx, CSourceLocation beginLoc, unsigned len);
    [[nodiscard]] const std::string& ToString() const;
    [[nodiscard]] CSourceLocation GetBeginningLocation() const { return BeginningLocation; }

private:
    CSourceLocation BeginningLocation;
    unsigned Length;
    std::string CachedString;
};

enum class EKind : uint16_t
{
    Node,
    File,
    CommandChildren,
    Command,
    CommandPost,
    PositionalArgument,
    NamedArgument,
    TransformArgument,
    CommandChildrenPost,
    Expr,
    Ident,
    Number,
    UnaryOp,
    BinaryOp,
    Vector,
    Call,
    WrappedExpr,
    ExprPost
};

/// Base class for all AST nodes
class ANode
{
public:
    static inline bool classof(const ANode*) { return true; }
    bool CanBeChild(ANode* node) const;
    void AddChild(ANode* node);
    /// Clears the children list, does not modify the actual text buffer in any way
    void ClearChildren();
    [[nodiscard]] EKind GetKind() const { return Kind; }
    [[nodiscard]] CToken* GetOpenToken() const { return Token; }
    [[nodiscard]] CToken* GetCloseToken() const { return CloseToken; }
    /// Returns the string representation, default implementation invokes ToTokenList.
    [[nodiscard]] virtual std::string ToString() const;
    /// Returns the tokens in order. Override for more specific AST node types.
    [[nodiscard]] virtual std::vector<CToken*> ToTokenList() const;

protected:
    ANode(EKind kind, EKind childKindBegin, EKind childKindEnd, CToken* token, CToken* closeToken = nullptr)
        : Kind(kind)
        , Token(token)
        , CloseToken(closeToken)
    {
        ChildKindRange = static_cast<uint32_t>(childKindBegin) | (static_cast<uint32_t>(childKindEnd) << 16);
    }

    EKind Kind;
    CToken* Token;
    CToken* CloseToken; // Optionally provide handling for matched open/close pairs
    std::vector<ANode*> Children;
    uint32_t ChildKindRange = 0;
};

//============================== Expressions ==============================

class IExprVisitor;

/// Base class for all expression types. Should not be used directly.
class AExpr : public ANode
{
public:
    std::any Accept(IExprVisitor* visitor);

    static inline bool classof(const ANode* other)
    {
        return other->GetKind() >= EKind::Expr && other->GetKind() < EKind::ExprPost;
    }

protected:
    AExpr(EKind kind, CToken* token, CToken* closeToken = nullptr)
        : ANode(kind, EKind::Expr, EKind::ExprPost, token, closeToken)
    {
    }
};

/// An identifier, like `abc123`
class AIdent : public AExpr
{
public:
    explicit AIdent(CToken* token)
        : AExpr(EKind::Ident, token)
    {
    }

    AIdent(CToken* dollar, CToken* token)
        : AExpr(EKind::Ident, dollar, token)
    {
    }

    static inline bool classof(const ANode* other) { return other->GetKind() == EKind::Ident; }

    /// For ident, ToString does not include any additional text like $
    [[nodiscard]] std::string ToString() const override;
};

/// A number, like `123.456`
class ANumber : public AExpr
{
public:
    explicit ANumber(CToken* token)
        : AExpr(EKind::Number, token)
    {
    }

    static inline bool classof(const ANode* other) { return other->GetKind() == EKind::Number; }

    [[nodiscard]] float AsFloat() const;
    [[nodiscard]] double AsDouble() const;
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

    static inline bool classof(const ANode* other) { return other->GetKind() == EKind::UnaryOp; }

    [[nodiscard]] EOperator GetOperatorType() const;
    [[nodiscard]] AExpr* GetOperand() const;
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

    static inline bool classof(const ANode* other) { return other->GetKind() == EKind::BinaryOp; }

    [[nodiscard]] EOperator GetOperatorType() const;
    [[nodiscard]] AExpr* GetLeft() const;
    [[nodiscard]] AExpr* GetRight() const;

    [[nodiscard]] std::vector<CToken*> ToTokenList() const override;
};

/// A list of items enclosed by a pair of parentheses, open and close tokens are the two parentheses
class AVector : public AExpr
{
public:
    AVector(CToken* openToken, CToken* closeToken)
        : AExpr(EKind::Vector, openToken, closeToken)
    {
    }

    static inline bool classof(const ANode* other) { return other->GetKind() == EKind::Vector; }

    [[nodiscard]] std::vector<AExpr*> GetItems() const;
};

/// A call expression in the form of `funcName argumentList`
class ACall : public AExpr
{
public:
    ACall(CToken* funcToken, AVector* argumentList);

    static inline bool classof(const ANode* other) { return other->GetKind() == EKind::Call; }

    [[nodiscard]] std::string GetFuncName() const { return Token->ToString(); }
    [[nodiscard]] AVector* GetOperandList() const { return cast<AVector>(Children[0]); }
};

/// Represents () or ${} in the context of an expression
class AWrappedExpr : public AExpr
{
public:
    AWrappedExpr(CToken* beginToken, CToken* endToken, CToken* secondToken, AExpr* expr)
        : AExpr(EKind::WrappedExpr, beginToken, endToken)
        , SecondToken(secondToken)
    {
        AddChild(expr);
    }

    static inline bool classof(const ANode* other) { return other->GetKind() == EKind::WrappedExpr; }

    [[nodiscard]] AExpr* GetExpr() const { return cast<AExpr>(Children[0]); }
    [[nodiscard]] std::vector<CToken*> ToTokenList() const override;

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

    static inline bool classof(const ANode* other) { return other->GetKind() == EKind::File; }

    [[nodiscard]] std::vector<ACommand*> GetCommands() const;
};

class APositionalArgument : public ANode
{
public:
    explicit APositionalArgument(AExpr* expression)
        : ANode(EKind::PositionalArgument, EKind::Expr, EKind::ExprPost, nullptr)
    {
        AddChild(expression);
    }

    static inline bool classof(const ANode* other) { return other->GetKind() == EKind::PositionalArgument; }

    [[nodiscard]] AExpr* GetExpr() const;
};

class ANamedArgument : public ANode
{
public:
    explicit ANamedArgument(CToken* nameToken)
        : ANode(EKind::NamedArgument, EKind::Expr, EKind::ExprPost, nameToken)
    {
    }

    static inline bool classof(const ANode* other) { return other->GetKind() == EKind::NamedArgument; }

    [[nodiscard]] std::string GetName() const;
    [[nodiscard]] AExpr* GetArgument(size_t index) const;
    [[nodiscard]] std::vector<AExpr*> GetArguments() const;
};

class ACommand : public ANode
{
public:
    ACommand(CToken* openToken, CToken* closeToken);

    static inline bool classof(const ANode* other) { return other->GetKind() == EKind::Command; }

    // Getters
    [[nodiscard]] std::string GetCommand() const { return Token->ToString(); }
    [[nodiscard]] std::string GetName() const { return GetPositionalArgument(0)->ToString(); }
    [[nodiscard]] APositionalArgument* GetPositionalArgument(size_t index) const;
    [[nodiscard]] ANamedArgument* GetNamedArgument(const std::string& name) const;
    [[nodiscard]] std::vector<ACommand*> GetSubCommands() const;
    [[nodiscard]] std::vector<ANamedArgument*> GetNamedArguments() const;
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

} // namespace Nome::AST
