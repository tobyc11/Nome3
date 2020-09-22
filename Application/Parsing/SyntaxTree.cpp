#include "SyntaxTree.h"

#include <algorithm>
#include <utility>

namespace Nome::AST
{

CToken::CToken(std::string text, unsigned int bufId, unsigned int start)
    : Text(std::move(text))
    , BufLoc { bufId, start }
{
}

std::string CToken::ToString() const { return Text; }

bool ANode::CanBeChild(ANode* node) const
{
    uint32_t i = ChildKindRange & 0xFFFF;
    uint32_t j = ChildKindRange >> 16;
    auto k = static_cast<uint32_t>(node->Kind);
    return !(k < i || k >= j);
}

void ANode::AddChild(ANode* node)
{
    if (!CanBeChild(node))
        throw CSemanticError("Node is not allowed to be added as a child", node);
    Children.push_back(node);
}

bool ANode::AddChildAfter(const ANode* after, ANode* child)
{
    if (!CanBeChild(child))
        throw CSemanticError("Node is not allowed to be added as a child", child);
    auto afterIter = std::find(Children.begin(), Children.end(), after);
    if (afterIter == Children.end())
        return false;
    Children.insert(afterIter, child);
    return true;
}

std::any AExpr::Accept(IExprVisitor* visitor)
{
    switch (Kind)
    {
    case EKind::Ident:
        return visitor->VisitIdent(static_cast<AIdent*>(this));
    case EKind::Number:
        return visitor->VisitNumber(static_cast<ANumber*>(this));
    case EKind::UnaryOp:
        return visitor->VisitUnaryOp(static_cast<AUnaryOp*>(this));
    case EKind::BinaryOp:
        return visitor->VisitBinaryOp(static_cast<ABinaryOp*>(this));
    case EKind::Call:
        return visitor->VisitCall(static_cast<ACall*>(this));
    case EKind::Vector:
        return visitor->VisitVector(static_cast<AVector*>(this));
    case EKind::WrappedExpr:
        return visitor->VisitWrappedExpr(static_cast<AWrappedExpr*>(this));
    default:
        break;
    }
    throw std::runtime_error("AExpr corruption detected in Accept(visitor)");
}

template <typename TFunc> class TGenericVisitor final : public IExprVisitor
{
public:
    explicit TGenericVisitor(TFunc& func)
        : Func(func)
    {
    }
    std::any VisitIdent(AIdent* sub) override { return Func(sub); }
    std::any VisitNumber(ANumber* sub) override { return Func(sub); }
    std::any VisitUnaryOp(AUnaryOp* sub) override { return Func(sub); }
    std::any VisitBinaryOp(ABinaryOp* sub) override { return Func(sub); }
    std::any VisitCall(ACall* sub) override { return Func(sub); }
    std::any VisitVector(AVector* sub) override { return Func(sub); }
    std::any VisitWrappedExpr(AWrappedExpr* sub) override { return Func(sub); }

private:
    TFunc& Func;
};

void AExpr::CollectTokens(std::vector<CToken*>& tokenList) const
{
    auto callSubType = [&](auto* subExpr) -> std::any {
        subExpr->CollectTokens(tokenList);
        return {};
    };
    TGenericVisitor visitor { callSubType };
    const_cast<AExpr*>(this)->Accept(&visitor);
}

std::ostream& operator<<(std::ostream& os, const AExpr& node)
{
    auto callOperatorOnSubType = [&](auto* subExpr) -> std::any {
        os << *subExpr;
        return nullptr;
    };
    TGenericVisitor visitor { callOperatorOnSubType };
    const_cast<AExpr&>(node).Accept(&visitor);
    return os;
}

float ANumber::AsFloat() const { return std::atof(Token->ToString().c_str()); }
double ANumber::AsDouble() const { return std::atof(Token->ToString().c_str()); }

static const std::map<std::string, AUnaryOp::EOperator> UnaryOpStringDecode = {
    { "-", AUnaryOp::EOperator::Neg }, { "+", AUnaryOp::EOperator::Plus }
};

AUnaryOp::EOperator AUnaryOp::GetOperatorType() const
{
    return UnaryOpStringDecode.at(Token->ToString());
}

AExpr* AUnaryOp::GetOperand() const
{
    // Type checking is done in ANode::AddChild, so in theory this shouldn't be a concern
    return static_cast<AExpr*>(Children[0]);
}

static const std::map<std::string, ABinaryOp::EOperator> BinaryOpStringDecode = {
    { "+", ABinaryOp::EOperator::Add },
    { "-", ABinaryOp::EOperator::Sub },
    { "/", ABinaryOp::EOperator::Div },
    { "^", ABinaryOp::EOperator::Exp },
    { "*", ABinaryOp::EOperator::Mul }
};

ABinaryOp::EOperator ABinaryOp::GetOperatorType() const
{
    return BinaryOpStringDecode.at(Token->ToString());
}

AExpr* ABinaryOp::GetLeft() const { return static_cast<AExpr*>(Children[0]); }

AExpr* ABinaryOp::GetRight() const { return static_cast<AExpr*>(Children[1]); }

std::vector<AExpr*> AVector::GetItems() const
{
    std::vector<AExpr*> exprs;
    for (ANode* child : Children)
        exprs.push_back(static_cast<AExpr*>(child));
    return exprs;
}

ACall::ACall(CToken* funcToken, AVector* argumentList)
    : AExpr(EKind::Call, funcToken)
{
    // Verify that the argument list only has one member
    if (argumentList->GetItems().size() > 1)
        throw CSemanticError("Call expression only supports zero or one argument", argumentList);
    AddChild(argumentList);
}

IExprVisitor::~IExprVisitor() { }

std::vector<ACommand*> AFile::GetCommands() const
{
    std::vector<ACommand*> cmds;
    for (ANode* child : Children)
        cmds.push_back(static_cast<ACommand*>(child));
    return cmds;
}

void AFile::CollectTokens(std::vector<CToken*>& tokenList) const
{
    for (auto* cmd : GetCommands())
        cmd->CollectTokens(tokenList);
}

std::string ANamedArgument::GetName() const { return Token->ToString(); }

AExpr* ANamedArgument::GetArgument(size_t index) const
{
    if (index >= Children.size())
        throw CSemanticError("Argument index out of bound for named argument", this);
    return static_cast<AExpr*>(Children[index]);
}

std::vector<AExpr*> ANamedArgument::GetArguments() const
{
    std::vector<AExpr*> exprs;
    for (ANode* child : Children)
        exprs.push_back(static_cast<AExpr*>(child));
    return exprs;
}

void ANamedArgument::CollectTokens(std::vector<CToken*>& tokenList) const
{
    tokenList.push_back(Token);
    for (auto* expr : GetArguments())
        expr->CollectTokens(tokenList);
}

ACommand::ACommand(CToken* openToken, CToken* closeToken)
    : ANode(EKind::Command, EKind::Command, EKind::CommandPost, openToken, closeToken)
{
}

// Color is an example of a named argument
void ACommand::AddNamedArgument(ANamedArgument* argument)
{
    auto iter = NamedArguments.find(argument->GetName());
    if (iter != NamedArguments.end()) 
        throw CSemanticError("Named argument is repeated.", argument);
    NamedArguments[argument->GetName()] = argument;
}

std::string ACommand::GetPositionalIdentAsString(size_t index) const
{
    if (!PositionalArguments.empty() && PositionalArguments.at(index)->GetKind() == EKind::Ident
        && PositionalArguments.size() > index)
    {
        return static_cast<AIdent*>(PositionalArguments.at(index))->ToString();
    }
    return {};
}

AExpr* ACommand::GetPositionalArgument(size_t index) const
{
    if (index >= PositionalArguments.size())
        return nullptr;
    return static_cast<AExpr*>(PositionalArguments[index]);
}

ANamedArgument* ACommand::GetNamedArgument(const std::string& name) const
{
    auto iter = NamedArguments.find(name);
    if (iter != NamedArguments.end())
        return iter->second;
    return nullptr;
}

std::vector<ACommand*> ACommand::GetSubCommands() const
{
    std::vector<ACommand*> result;
    for (auto* node : Children)
        result.push_back(static_cast<ACommand*>(node));
    return result;
}

void ACommand::CollectTokens(std::vector<CToken*>& tokenList) const
{
    tokenList.push_back(GetOpenToken());
    for (auto* expr : PositionalArguments)
        expr->CollectTokens(tokenList);
    for (auto* arg : Transforms)
        arg->CollectTokens(tokenList);
    for (auto [name, arg] : NamedArguments)
        arg->CollectTokens(tokenList);
    for (auto* sub : GetSubCommands())
        sub->CollectTokens(tokenList);
    if (GetCloseToken())
        tokenList.push_back(GetCloseToken());
}

std::ostream& operator<<(std::ostream& os, const AFile& node)
{
    for (ACommand* cmd : node.GetCommands())
        os << *cmd << std::endl;
    return os;
}

std::ostream& operator<<(std::ostream& os, const ANamedArgument& node)
{
    os << node.Token->ToString();
    for (AExpr* expr : node.GetArguments())
        os << ":" << *expr;
    return os;
}

std::ostream& operator<<(std::ostream& os, const ACommand& node)
{
    os << node.Token->ToString();
    int i = 0;
    for (auto* expr : node.PositionalArguments)
    {
        os << " " << i << ":" << *expr;
        i++;
    }
    for (auto* expr : node.Transforms)
        os << " " << *expr;
    for (const auto& pair : node.NamedArguments)
        os << " " << *pair.second;
    os << " " << node.CloseToken->ToString();
    return os;
}

const char* CSemanticError::what() const noexcept { return Message.c_str(); }

}
