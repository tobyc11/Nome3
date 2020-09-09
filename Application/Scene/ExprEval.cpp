#include "ExprEval.h"

namespace Nome::Scene
{

std::any CExprEvalDirect::VisitIdent(AST::AIdent* ident)
{
    throw AST::CSemanticError("Direct expression does not support variable references", ident);
    return 0.0f;
}

std::any CExprEvalDirect::VisitNumber(AST::ANumber* number) { return number->AsFloat(); }

std::any CExprEvalDirect::VisitUnaryOp(AST::AUnaryOp* op)
{
    auto operand = std::any_cast<float>(op->GetOperand()->Accept(this));
    switch (op->GetOperatorType())
    {
    case AST::AUnaryOp::EOperator::Neg:
        return -operand;
    case AST::AUnaryOp::EOperator::Plus:
        return operand;
    }
}

std::any CExprEvalDirect::VisitBinaryOp(AST::ABinaryOp* op)
{
    auto lhs = std::any_cast<float>(op->GetLeft()->Accept(this));
    auto rhs = std::any_cast<float>(op->GetRight()->Accept(this));
    switch (op->GetOperatorType())
    {
    case AST::ABinaryOp::EOperator::Add:
        return lhs + rhs;
    case AST::ABinaryOp::EOperator::Sub:
        return lhs - rhs;
    case AST::ABinaryOp::EOperator::Mul:
        return lhs * rhs;
    case AST::ABinaryOp::EOperator::Div:
        return lhs / rhs;
    case AST::ABinaryOp::EOperator::Exp:
        return std::pow(lhs, rhs);
    }
}

std::any CExprEvalDirect::VisitCall(AST::ACall* call)
{
    auto operand = std::any_cast<float>(call->GetOperandList()->Accept(this));
    std::string func = call->GetFuncName();
    if (func == "sin")
        return std::sin(operand);
    else if (func == "cos")
        return std::cos(operand);
    throw AST::CSemanticError("Called undefined function", call);
}

std::any CExprEvalDirect::VisitVector(AST::AVector* vector)
{
    auto items = vector->GetItems();
    if (items.size() != 1)
        throw AST::CSemanticError("Vector size != 1, can't evaluate", vector);
    return items[0]->Accept(this);
}

std::any CExprEvalDirect::VisitWrappedExpr(AST::AWrappedExpr* expr)
{
    return expr->GetExpr()->Accept(this);
}

}
