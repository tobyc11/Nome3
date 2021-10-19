#include "SyntaxTreeBuilder.h"
#include "SyntaxTree.h"

namespace Nome
{

using namespace AST;

antlrcpp::Any CFileBuilder::visitFile(NomParser::FileContext* context)
{
    auto* file = new (*Ctx) AFile();
    for (auto* command : context->generalizedCommand())
        file->AddChild(this->visit(command).as<ANode*>());
    return file;
}

antlrcpp::Any CFileBuilder::visitArgClosed(NomParser::ArgClosedContext* context)
{
    return new (*Ctx) ANamedArgument(ConvertToken(context->getStart()));
}

antlrcpp::Any CFileBuilder::visitArgHidden(NomParser::ArgHiddenContext* context)
{
    return new (*Ctx) ANamedArgument(ConvertToken(context->getStart()));
}

antlrcpp::Any CFileBuilder::visitArgSurface(NomParser::ArgSurfaceContext* context)
{
    auto* arg = new (*Ctx) ANamedArgument(ConvertToken(context->getStart()));
    arg->AddChild(visit(context->ident()).as<AExpr*>());
    return arg;
}

antlrcpp::Any CFileBuilder::visitArgSlices(NomParser::ArgSlicesContext* context)
{
    auto* result = new (*Ctx) ANamedArgument(ConvertToken(context->getStart()));
    result->AddChild(visit(context->expression()).as<AExpr*>());
    return result;
}

antlrcpp::Any CFileBuilder::visitArgOrder(NomParser::ArgOrderContext* context)
{
    auto* result = new (*Ctx) ANamedArgument(ConvertToken(context->getStart()));
    result->AddChild(visit(context->expression()).as<AExpr*>());
    return result;
}

antlrcpp::Any CFileBuilder::visitArgTransformTwo(NomParser::ArgTransformTwoContext* context)
{
    auto* result = new (*Ctx) ANamedArgument(ConvertToken(context->getStart()));
    auto* list = new (*Ctx) AVector(ConvertToken(context->LPAREN(0)), ConvertToken(context->RPAREN(0)));
    list->AddChild(visit(context->exp1).as<AExpr*>());
    list->AddChild(visit(context->exp2).as<AExpr*>());
    list->AddChild(visit(context->exp3).as<AExpr*>());
    result->AddChild(list);
    list = new (*Ctx) AVector(ConvertToken(context->LPAREN(1)), ConvertToken(context->RPAREN(1)));
    list->AddChild(visit(context->exp4).as<AExpr*>());
    result->AddChild(list);
    return result;
}

antlrcpp::Any CFileBuilder::visitArgTransformOne(NomParser::ArgTransformOneContext* context)
{
    auto* result = new (*Ctx) ANamedArgument(ConvertToken(context->getStart()));
    auto* firstList = new (*Ctx) AVector(ConvertToken(context->LPAREN()), ConvertToken(context->RPAREN()));
    for (auto* expr : context->expression())
        firstList->AddChild(visit(expr).as<AExpr*>());
    result->AddChild(firstList);
    return result;
}

antlrcpp::Any CFileBuilder::visitArgColor(NomParser::ArgColorContext* context)
{
    auto* result = new (*Ctx) ANamedArgument(ConvertToken(context->getStart()));
    auto* colorList =
        new (*Ctx) AVector(ConvertToken(context->LPAREN()->getSymbol()), ConvertToken(context->RPAREN()->getSymbol()));
    for (auto* expr : context->expression())
    {
        colorList->AddChild(visit(expr).as<AExpr*>());
    }
    result->AddChild(colorList);
    return result;
}

antlrcpp::Any CFileBuilder::visitPositionalArg(NomParser::PositionalArgContext* context)
{
    auto* expr = visitChildren(context).as<AExpr*>();
    auto* posArg = new (*Ctx) APositionalArgument(expr);
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
        return (ANode*)visit(context->positionalArg()).as<APositionalArgument*>();
    else
        return (ANode*)visit(context->namedArg()).as<ANamedArgument*>();
}

antlrcpp::Any CFileBuilder::visitSetCommand(NomParser::SetCommandContext* context)
{
    auto* cmd = new (*Ctx) ACommand(ConvertToken(context->KW_SET()), nullptr);
    cmd->AddChild(new (*Ctx) APositionalArgument(visit(context->ident()).as<AExpr*>()));
    for (auto* expr : context->expression())
        cmd->AddChild(new (*Ctx) APositionalArgument(visit(expr).as<AExpr*>()));
    return (ANode*)cmd;
}

antlrcpp::Any CFileBuilder::visitGeneralizedCommand(NomParser::GeneralizedCommandContext* context)
{
    CToken* beginTok = nullptr;
    if (context->KW_CMD())
        beginTok = ConvertToken(context->KW_CMD());
    else if (context->KW_SURFACE())
        beginTok = ConvertToken(context->KW_SURFACE());
    assert(beginTok);
    auto* cmd = new (*Ctx) ACommand(beginTok, ConvertToken(context->KW_END_CMD()));
    for (auto* anyArg : context->anyArg())
        cmd->AddChild(visit(anyArg).as<ANode*>());
    for (auto* subCmd : context->generalizedCommand())
        cmd->AddChild(visit(subCmd).as<ANode*>());
    for (auto* setCmd : context->setCommand())
        cmd->AddChild(visit(setCmd).as<ANode*>());
    return (ANode*)cmd;
}

antlrcpp::Any CFileBuilder::visitCall(NomParser::CallContext* context)
{
    auto* argList = new (*Ctx) AVector(ConvertToken(context->LPAREN()), ConvertToken(context->RPAREN()));
    argList->AddChild(visit(context->expression()).as<AExpr*>());
    auto* funcToken = ConvertToken(context->ident()->IDENT());
    return static_cast<AExpr*>(new (*Ctx) ACall(funcToken, argList));
}

antlrcpp::Any CFileBuilder::visitUnaryOp(NomParser::UnaryOpContext* context)
{
    if (context->PLUS())
        return static_cast<AExpr*>(
            new (*Ctx) AUnaryOp(ConvertToken(context->PLUS()), visit(context->expression()).as<AExpr*>()));
    if (context->MINUS())
        return static_cast<AExpr*>(
            new (*Ctx) AUnaryOp(ConvertToken(context->MINUS()), visit(context->expression()).as<AExpr*>()));
    throw CSemanticError("Invalid unary operator", nullptr);
}

antlrcpp::Any CFileBuilder::visitSubExpParen(NomParser::SubExpParenContext* context)
{
    return static_cast<AExpr*>(new (*Ctx) AWrappedExpr(ConvertToken(context->LPAREN()), ConvertToken(context->RPAREN()),
                                                       nullptr, visit(context->expression()).as<AExpr*>()));
}

antlrcpp::Any CFileBuilder::visitSubExpCurly(NomParser::SubExpCurlyContext* context)
{
    return static_cast<AExpr*>(new (*Ctx)
                                   AWrappedExpr(ConvertToken(context->beg), ConvertToken(context->end),
                                                ConvertToken(context->sec), visit(context->expression()).as<AExpr*>()));
}

antlrcpp::Any CFileBuilder::visitBinOp(NomParser::BinOpContext* context)
{

    auto left = visit(context->expression(0)).as<AExpr*>();
    auto right = visit(context->expression(1)).as<AExpr*>();
    auto* op = ConvertToken(context->op);
    return static_cast<AExpr*>(new (*Ctx) ABinaryOp(op, left, right));
}

antlrcpp::Any CFileBuilder::visitScientific(NomParser::ScientificContext* context)
{
    return static_cast<AExpr*>(new (*Ctx) ANumber(ConvertToken(context->SCIENTIFIC_NUMBER())));
}

antlrcpp::Any CFileBuilder::visitIdent(NomParser::IdentContext* context)
{
    if (context->DOLLAR())
        return static_cast<AExpr*>(new (*Ctx) AIdent(ConvertToken(context->DOLLAR()), ConvertToken(context->IDENT())));
    return static_cast<AExpr*>(new (*Ctx) AIdent(ConvertToken(context->IDENT())));
}

antlrcpp::Any CFileBuilder::visitAtomExpr(NomParser::AtomExprContext* context) { return visit(context->atom()); }

antlrcpp::Any CFileBuilder::visitVector(NomParser::VectorContext* context)
{
    auto* vectorNode = new (*Ctx) AVector(ConvertToken(context->LPAREN()), ConvertToken(context->RPAREN()));
    for (auto* expr : context->expression())
        vectorNode->AddChild(visit(expr).as<AExpr*>());
    return static_cast<AExpr*>(vectorNode);
}

CToken* CFileBuilder::ConvertToken(antlr4::Token* token)
{
    auto start = token->getStartIndex();
    auto beginLoc = Ctx->GetSourceMgr()->GlobalToBufOffset(start);
    auto len = token->getStopIndex() - start + 1;
    return CToken::Create(*Ctx, beginLoc, len);
}

CToken* CFileBuilder::ConvertToken(antlr4::tree::TerminalNode* token) { return ConvertToken(token->getSymbol()); }

}
