#include "SyntaxTreeBuilder.h"
#include "SyntaxTree.h"

namespace Nome
{

antlrcpp::Any CFileBuilder::visitFile(NomParser::FileContext* context)
{
    auto* file = new (*Ctx) AST::AFile();
    for (auto* command : context->generalizedCommand())
        file->AddChild(this->visit(command));
    return file;
}

antlrcpp::Any CFileBuilder::visitArgClosed(NomParser::ArgClosedContext* context)
{
    return new (*Ctx) AST::ANamedArgument(ConvertToken(context->getStart()));
}

antlrcpp::Any CFileBuilder::visitArgHidden(NomParser::ArgHiddenContext* context)
{
    return new (*Ctx) AST::ANamedArgument(ConvertToken(context->getStart()));
}

antlrcpp::Any CFileBuilder::visitArgSurface(NomParser::ArgSurfaceContext* context)
{
    auto* arg = new (*Ctx) AST::ANamedArgument(ConvertToken(context->getStart()));
    arg->AddChild(visit(context->ident()).as<AST::AExpr*>());
    return arg;
}

antlrcpp::Any CFileBuilder::visitArgSlices(NomParser::ArgSlicesContext* context)
{
    auto* result = new (*Ctx) AST::ANamedArgument(ConvertToken(context->getStart()));
    result->AddChild(visit(context->expression()).as<AST::AExpr*>());
    return result;
}

antlrcpp::Any CFileBuilder::visitArgOrder(NomParser::ArgOrderContext* context)
{
    auto* result = new (*Ctx) AST::ANamedArgument(ConvertToken(context->getStart()));
    result->AddChild(visit(context->expression()).as<AST::AExpr*>());
    return result;
}

antlrcpp::Any CFileBuilder::visitArgTransformTwo(NomParser::ArgTransformTwoContext* context)
{
    auto* result = new (*Ctx) AST::ANamedArgument(ConvertToken(context->getStart()));
    auto* list = new (*Ctx) AST::AVector(ConvertToken(context->LPAREN(0)), ConvertToken(context->RPAREN(0)));
    list->AddChild(visit(context->exp1).as<AST::AExpr*>());
    list->AddChild(visit(context->exp2).as<AST::AExpr*>());
    list->AddChild(visit(context->exp3).as<AST::AExpr*>());
    result->AddChild(list);
    list = new (*Ctx) AST::AVector(ConvertToken(context->LPAREN(1)), ConvertToken(context->RPAREN(1)));
    list->AddChild(visit(context->exp4).as<AST::AExpr*>());
    result->AddChild(list);
    return result;
}

antlrcpp::Any CFileBuilder::visitArgTransformOne(NomParser::ArgTransformOneContext* context)
{
    auto* result = new (*Ctx) AST::ANamedArgument(ConvertToken(context->getStart()));
    auto* firstList = new (*Ctx) AST::AVector(ConvertToken(context->LPAREN()), ConvertToken(context->RPAREN()));
    for (auto* expr : context->expression())
        firstList->AddChild(visit(expr).as<AST::AExpr*>());
    result->AddChild(firstList);
    return result;
}

antlrcpp::Any CFileBuilder::visitArgColor(NomParser::ArgColorContext* context)
{
    auto* result = new (*Ctx) AST::ANamedArgument(ConvertToken(context->getStart()));
    auto* colorList = new (*Ctx)
        AST::AVector(ConvertToken(context->LPAREN()->getSymbol()), ConvertToken(context->RPAREN()->getSymbol()));
    for (auto* expr : context->expression())
    {
        colorList->AddChild(visit(expr).as<AST::AExpr*>());
    }
    result->AddChild(colorList);
    return result;
}

antlrcpp::Any CFileBuilder::visitPositionalArg(NomParser::PositionalArgContext* context)
{
    auto* expr = visitChildren(context).as<AST::AExpr*>();
    auto* posArg = new (*Ctx) AST::APositionalArgument(expr);
    return posArg;
}

antlrcpp::Any CFileBuilder::visitNamedArg(NomParser::NamedArgContext* context)
{
    // The default version of visitChildren just keeps the last result.
    // Since namedArg only has one child, this should work.
    return visitChildren(context);
}

antlrcpp::Any CFileBuilder::visitAnyArg(NomParser::AnyArgContext* context)
{
    if (context->positionalArg())
        return (AST::ANode*)visit(context->positionalArg()).as<AST::APositionalArgument*>();
    else
        return (AST::ANode*)visit(context->namedArg()).as<AST::ANamedArgument*>();
}

antlrcpp::Any CFileBuilder::visitSetCommand(NomParser::SetCommandContext* context)
{
    auto* cmd = new (*Ctx) AST::ACommand(ConvertToken(context->KW_SET()), nullptr);
    cmd->AddChild(new (*Ctx) AST::APositionalArgument(visit(context->ident()).as<AST::AExpr*>()));
    for (auto* expr : context->expression())
        cmd->AddChild(new (*Ctx) AST::APositionalArgument(visit(expr).as<AST::AExpr*>()));
    return (AST::ANode*)cmd;
}

antlrcpp::Any CFileBuilder::visitGeneralizedCommand(NomParser::GeneralizedCommandContext* context)
{
    AST::CToken* beginTok = nullptr;
    if (context->KW_CMD())
        beginTok = ConvertToken(context->KW_CMD());
    else if (context->KW_SURFACE())
        beginTok = ConvertToken(context->KW_SURFACE());
    assert(beginTok);
    auto* cmd = new (*Ctx) AST::ACommand(beginTok, ConvertToken(context->KW_END_CMD()));
    for (auto* anyArg : context->anyArg())
        cmd->AddChild(visit(anyArg));
    for (auto* subCmd : context->generalizedCommand())
        cmd->AddChild(visit(subCmd));
    for (auto* setCmd : context->setCommand())
        cmd->AddChild(visit(setCmd));
    return (AST::ANode*)cmd;
}

antlrcpp::Any CFileBuilder::visitCall(NomParser::CallContext* context)
{
    auto* argList = new (*Ctx) AST::AVector(ConvertToken(context->LPAREN()), ConvertToken(context->RPAREN()));
    argList->AddChild(visit(context->expression()).as<AST::AExpr*>());
    auto* funcToken = ConvertToken(context->ident()->IDENT());
    return static_cast<AST::AExpr*>(new (*Ctx) AST::ACall(funcToken, argList));
}

antlrcpp::Any CFileBuilder::visitUnaryOp(NomParser::UnaryOpContext* context)
{
    if (context->PLUS())
        return static_cast<AST::AExpr*>(new (*Ctx)
                                            AST::AUnaryOp(ConvertToken(context->PLUS()), visit(context->expression())));
    if (context->MINUS())
        return static_cast<AST::AExpr*>(
            new (*Ctx) AST::AUnaryOp(ConvertToken(context->MINUS()), visit(context->expression())));
    throw AST::CSemanticError("Invalid unary operator", nullptr);
}

antlrcpp::Any CFileBuilder::visitSubExpParen(NomParser::SubExpParenContext* context)
{
    return static_cast<AST::AExpr*>(new (*Ctx) AST::AWrappedExpr(
        ConvertToken(context->LPAREN()), ConvertToken(context->RPAREN()), nullptr, visit(context->expression())));
}

antlrcpp::Any CFileBuilder::visitSubExpCurly(NomParser::SubExpCurlyContext* context)
{
    return static_cast<AST::AExpr*>(new (*Ctx)
                                        AST::AWrappedExpr(ConvertToken(context->beg), ConvertToken(context->end),
                                                          ConvertToken(context->sec), visit(context->expression())));
}

antlrcpp::Any CFileBuilder::visitBinOp(NomParser::BinOpContext* context)
{

    auto left = visit(context->expression(0));
    auto right = visit(context->expression(1));
    auto* op = ConvertToken(context->op);
    return static_cast<AST::AExpr*>(new (*Ctx) AST::ABinaryOp(op, left, right));
}

antlrcpp::Any CFileBuilder::visitScientific(NomParser::ScientificContext* context)
{
    return static_cast<AST::AExpr*>(new (*Ctx) AST::ANumber(ConvertToken(context->SCIENTIFIC_NUMBER())));
}

antlrcpp::Any CFileBuilder::visitIdent(NomParser::IdentContext* context)
{
    return static_cast<AST::AExpr*>(new (*Ctx) AST::AIdent(ConvertToken(context->IDENT())));
}

antlrcpp::Any CFileBuilder::visitAtomExpr(NomParser::AtomExprContext* context) { return visit(context->atom()); }

antlrcpp::Any CFileBuilder::visitVector(NomParser::VectorContext* context)
{
    auto* vectorNode = new (*Ctx) AST::AVector(ConvertToken(context->LPAREN()), ConvertToken(context->RPAREN()));
    for (auto* expr : context->expression())
        vectorNode->AddChild(visit(expr).as<AST::AExpr*>());
    return static_cast<AST::AExpr*>(vectorNode);
}

AST::CToken* CFileBuilder::ConvertToken(antlr4::Token* token)
{
    auto start = token->getStartIndex();
    auto beginLoc = Ctx->GetSourceMgr()->GlobalToBufOffset(start);
    auto len = token->getStopIndex() - start + 1;
    return AST::CToken::Create(*Ctx, beginLoc, len);
}

AST::CToken* CFileBuilder::ConvertToken(antlr4::tree::TerminalNode* token)
{
    return ConvertToken(token->getSymbol());
}

}
