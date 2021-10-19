#pragma once
#include "ASTContext.h"
#include "NomBaseVisitor.h"

namespace Nome
{

/// CFileBuilder builds the AST from an ANTLR parse tree FileContext
class CFileBuilder : public NomBaseVisitor
{
public:
    explicit CFileBuilder(AST::CASTContext* ctx)
        : Ctx(ctx)
    {
    }

    antlrcpp::Any visitFile(NomParser::FileContext* context) override;

    // Functions below return ANamedArg*
    antlrcpp::Any visitArgClosed(NomParser::ArgClosedContext* context) override;
    antlrcpp::Any visitArgHidden(NomParser::ArgHiddenContext* context) override;
    antlrcpp::Any visitArgSurface(NomParser::ArgSurfaceContext* context) override;
    antlrcpp::Any visitArgSlices(NomParser::ArgSlicesContext* context) override;
    antlrcpp::Any visitArgOrder(NomParser::ArgOrderContext* context) override;
    antlrcpp::Any visitArgTransformTwo(NomParser::ArgTransformTwoContext* context) override;
    antlrcpp::Any visitArgTransformOne(NomParser::ArgTransformOneContext* context) override;
    antlrcpp::Any visitArgColor(NomParser::ArgColorContext* context) override;

    // Return type is APositionalArg*
    antlrcpp::Any visitPositionalArg(NomParser::PositionalArgContext* context) override;
    // Return type is ANamedArg*
    antlrcpp::Any visitNamedArg(NomParser::NamedArgContext* context) override;
    // Return type is ANode*
    antlrcpp::Any visitAnyArg(NomParser::AnyArgContext* context) override;

    // Return type is ANode*
    antlrcpp::Any visitSetCommand(NomParser::SetCommandContext* context) override;
    antlrcpp::Any visitGeneralizedCommand(NomParser::GeneralizedCommandContext* context) override;

    // They should all return AExpr*
    antlrcpp::Any visitCall(NomParser::CallContext* context) override;
    antlrcpp::Any visitUnaryOp(NomParser::UnaryOpContext* context) override;
    antlrcpp::Any visitSubExpParen(NomParser::SubExpParenContext* context) override;
    antlrcpp::Any visitSubExpCurly(NomParser::SubExpCurlyContext* context) override;
    antlrcpp::Any visitBinOp(NomParser::BinOpContext* context) override;
    antlrcpp::Any visitAtom(NomParser::AtomContext* context) override { return visitChildren(context); }
    antlrcpp::Any visitScientific(NomParser::ScientificContext* context) override;
    antlrcpp::Any visitIdent(NomParser::IdentContext* context) override;
    antlrcpp::Any visitAtomExpr(NomParser::AtomExprContext* context) override;
    antlrcpp::Any visitVector(NomParser::VectorContext* context) override;

private:
    AST::CToken* ConvertToken(antlr4::Token* token);
    AST::CToken* ConvertToken(antlr4::tree::TerminalNode* token);

    AST::CASTContext* Ctx;
};

} // namespace Nome
