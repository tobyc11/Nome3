#include "SyntaxTree.h"

#include <algorithm>
#include <stdexcept>
#include <unordered_map>

namespace Nome::AST
{

CToken::CToken(CSourceLocation beginLoc, unsigned len)
    : BeginningLocation(beginLoc)
    , Length(len)
{
}

CToken* CToken::Create(CASTContext& ctx, CSourceLocation beginLoc, unsigned len)
{
    auto* r = new (ctx) CToken(beginLoc, len);

    size_t fileOff = ctx.GetSourceMgr()->GetFileOffset(beginLoc).value();
    // TODO: Supremely inefficient
    auto text = ctx.GetSourceMgr()->CollectText();
    r->CachedString = text.substr(fileOff, len);

    return r;
}

const std::string& CToken::ToString() const
{
    const static std::string errorString("<TOKEN ERROR>");
    if (CachedString.length() == Length)
        return CachedString;
    return errorString;
}

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

void ANode::ClearChildren() { Children.clear(); }

std::string ANode::ToString() const
{
    std::string result;
    for (const auto* token : this->ToTokenList())
    {
        // TODO: right now we always insert a space between. There should be something we can do.
        if (!result.empty())
            result += " ";
        result += token->ToString();
    }
    return result;
}

std::vector<CToken*> ANode::ToTokenList() const
{
    std::vector<CToken*> result;
    if (GetOpenToken())
        result.push_back(GetOpenToken());
    for (const auto* child : Children)
    {
        auto childResult = child->ToTokenList();
        result.insert(result.end(), childResult.begin(), childResult.end());
    }
    if (GetCloseToken())
        result.push_back(GetCloseToken());
    return result;
}

std::any AExpr::Accept(IExprVisitor* visitor)
{
    switch (Kind)
    {
    case EKind::Ident:
        return visitor->VisitIdent(cast<AIdent>(this));
    case EKind::Number:
        return visitor->VisitNumber(cast<ANumber>(this));
    case EKind::UnaryOp:
        return visitor->VisitUnaryOp(cast<AUnaryOp>(this));
    case EKind::BinaryOp:
        return visitor->VisitBinaryOp(cast<ABinaryOp>(this));
    case EKind::Call:
        return visitor->VisitCall(cast<ACall>(this));
    case EKind::Vector:
        return visitor->VisitVector(cast<AVector>(this));
    case EKind::WrappedExpr:
        return visitor->VisitWrappedExpr(cast<AWrappedExpr>(this));
    default:
        break;
    }
    throw std::logic_error("AExpr corruption detected in Accept(visitor)");
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

float ANumber::AsFloat() const { return static_cast<float>(std::atof(Token->ToString().c_str())); }
double ANumber::AsDouble() const { return std::atof(Token->ToString().c_str()); }

static const std::map<std::string, AUnaryOp::EOperator> UnaryOpStringDecode = { { "-", AUnaryOp::EOperator::Neg },
                                                                                { "+", AUnaryOp::EOperator::Plus } };

AUnaryOp::EOperator AUnaryOp::GetOperatorType() const { return UnaryOpStringDecode.at(Token->ToString()); }

AExpr* AUnaryOp::GetOperand() const
{
    // Type checking is done in ANode::AddChild, so in theory this shouldn't be a concern
    return cast<AExpr>(Children[0]);
}

static const std::map<std::string, ABinaryOp::EOperator> BinaryOpStringDecode = { { "+", ABinaryOp::EOperator::Add },
                                                                                  { "-", ABinaryOp::EOperator::Sub },
                                                                                  { "/", ABinaryOp::EOperator::Div },
                                                                                  { "^", ABinaryOp::EOperator::Exp },
                                                                                  { "*", ABinaryOp::EOperator::Mul } };

ABinaryOp::EOperator ABinaryOp::GetOperatorType() const { return BinaryOpStringDecode.at(Token->ToString()); }

AExpr* ABinaryOp::GetLeft() const { return cast<AExpr>(Children[0]); }

AExpr* ABinaryOp::GetRight() const { return cast<AExpr>(Children[1]); }

std::vector<CToken*> ABinaryOp::ToTokenList() const
{
    assert(Children.size() == 2 && "A binary operator must have 2 children.");
    std::vector<CToken*> result;
    auto leftTokens = Children[0]->ToTokenList();
    result.insert(result.begin(), leftTokens.begin(), leftTokens.end());
    result.push_back(GetOpenToken());
    auto rightTokens = Children[1]->ToTokenList();
    result.insert(result.begin(), rightTokens.begin(), rightTokens.end());
    return result;
}

std::vector<AExpr*> AVector::GetItems() const
{
    std::vector<AExpr*> exprs;
    for (ANode* child : Children)
        exprs.push_back(cast<AExpr>(child));
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

std::vector<CToken*> AWrappedExpr::ToTokenList() const
{
    // Similar to ANode::ToTokenList, except we also consider SecondToken
    std::vector<CToken*> result = { GetOpenToken() };
    if (SecondToken)
        result.push_back(SecondToken);
    for (const auto* child : Children)
    {
        auto childResult = child->ToTokenList();
        result.insert(result.end(), childResult.begin(), childResult.end());
    }
    if (GetCloseToken())
    {
        result.push_back(GetCloseToken());
    }
    return result;
}

IExprVisitor::~IExprVisitor() = default;

std::vector<ACommand*> AFile::GetCommands() const
{
    std::vector<ACommand*> cmds;
    for (ANode* child : Children)
        cmds.push_back(cast<ACommand>(child));
    return cmds;
}

AExpr* APositionalArgument::GetExpr() const { return cast<AExpr>(Children[0]); }

std::string ANamedArgument::GetName() const { return Token->ToString(); }

AExpr* ANamedArgument::GetArgument(size_t index) const
{
    if (index >= Children.size())
        throw CSemanticError("Argument index out of bound for named argument", this);
    return cast<AExpr>(Children[index]);
}

std::vector<AExpr*> ANamedArgument::GetArguments() const
{
    std::vector<AExpr*> exprs;
    for (ANode* child : Children)
        exprs.push_back(cast<AExpr>(child));
    return exprs;
}

ACommand::ACommand(CToken* openToken, CToken* closeToken)
    : ANode(EKind::Command, EKind::CommandChildren, EKind::CommandChildrenPost, openToken, closeToken)
{
}

APositionalArgument* ACommand::GetPositionalArgument(size_t index) const
{
    size_t i = 0;
    for (auto* child : Children)
        if (isa<APositionalArgument>(child))
        {
            if (i == index)
                return cast<APositionalArgument>(child);
            ++i;
        }
    return nullptr;
}

ANamedArgument* ACommand::GetNamedArgument(const std::string& name) const
{
    std::unordered_map<std::string, ANamedArgument*> result;
    for (auto* child : Children)
        if (auto* filteredChild = dyn_cast<ANamedArgument>(child))
            result.emplace(filteredChild->GetName(), filteredChild);
    return result[name];
}

std::vector<ACommand*> ACommand::GetSubCommands() const
{
    std::vector<ACommand*> result;
    for (auto* child : Children)
        if (auto* filteredChild = dyn_cast<ACommand>(child))
            result.push_back(filteredChild);
    return result;
}

std::vector<ANamedArgument*> ACommand::GetNamedArguments() const
{
    std::vector<ANamedArgument*> result;
    for (auto* child : Children)
        if (auto* filteredChild = dyn_cast<ANamedArgument>(child))
            result.push_back(filteredChild);
    return result;
}

const char* CSemanticError::what() const noexcept { return Message.c_str(); }

}
