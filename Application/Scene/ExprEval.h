#pragma once
#include <Parsing/SyntaxTree.h>
#include <cmath>

namespace Nome::Scene
{

class CExprEvalDirect : public AST::IExprVisitor
{
public:
    std::any VisitIdent(AST::AIdent* ident) override;
    std::any VisitNumber(AST::ANumber* number) override;
    std::any VisitUnaryOp(AST::AUnaryOp* op) override;
    std::any VisitBinaryOp(AST::ABinaryOp* op) override;
    std::any VisitCall(AST::ACall* call) override;
    std::any VisitVector(AST::AVector* vector) override;
    std::any VisitWrappedExpr(AST::AWrappedExpr* expr) override;
};

}
