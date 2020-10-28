#include "SyntaxTreeBuilder.h"

namespace Nome
{

antlrcpp::Any CFileBuilder::visitFile(NomParser::FileContext* context)
{
    AST::AFile* file = new AST::AFile();
    for (auto* command : context->command())
        file->AddChild(this->visit(command).as<AST::ACommand*>());
    return file;
}

antlrcpp::Any CFileBuilder::visitArgClosed(NomParser::ArgClosedContext* context)
{
    AST::ANamedArgument* arg = new AST::ANamedArgument(ConvertToken(context->getStart()));
    return arg;
}

antlrcpp::Any CFileBuilder::visitArgHidden(NomParser::ArgHiddenContext* context)
{
    AST::ANamedArgument* arg = new AST::ANamedArgument(ConvertToken(context->getStart()));
    return arg;
}

antlrcpp::Any CFileBuilder::visitArgTime(NomParser::ArgTimeContext* context)
{
    AST::ANamedArgument* arg = new AST::ANamedArgument(ConvertToken(context->getStart()));
    return arg;
}

antlrcpp::Any CFileBuilder::visitArgFrame(NomParser::ArgFrameContext* context)
{
    AST::ANamedArgument* arg = new AST::ANamedArgument(ConvertToken(context->getStart()));
    return arg;
}

antlrcpp::Any CFileBuilder::visitArgSurface(NomParser::ArgSurfaceContext* context)
{
    AST::ANamedArgument* arg = new AST::ANamedArgument(ConvertToken(context->getStart()));
    arg->AddChild(visit(context->ident()).as<AST::AExpr*>());
    return arg;
}

antlrcpp::Any CFileBuilder::visitArgSlices(NomParser::ArgSlicesContext* context)
{
    auto* result = new AST::ANamedArgument(ConvertToken(context->getStart()));
    result->AddChild(visit(context->expression()).as<AST::AExpr*>());
    return result;
}

antlrcpp::Any CFileBuilder::visitArgOrder(NomParser::ArgOrderContext* context)
{
    auto* result = new AST::ANamedArgument(ConvertToken(context->getStart()));
    result->AddChild(visit(context->expression()).as<AST::AExpr*>());
    return result;
}

antlrcpp::Any CFileBuilder::visitArgTransformTwo(NomParser::ArgTransformTwoContext* context)
{
    auto* result = new AST::ANamedArgument(ConvertToken(context->getStart()));
    auto* list =
        new AST::AVector(ConvertToken(context->LPAREN(0)), ConvertToken(context->RPAREN(0)));
    list->AddChild(visit(context->exp1).as<AST::AExpr*>());
    list->AddChild(visit(context->exp2).as<AST::AExpr*>());
    list->AddChild(visit(context->exp3).as<AST::AExpr*>());
    result->AddChild(list);
    list = new AST::AVector(ConvertToken(context->LPAREN(1)), ConvertToken(context->RPAREN(1)));
    list->AddChild(visit(context->exp4).as<AST::AExpr*>());
    result->AddChild(list);
    return result;
}

antlrcpp::Any CFileBuilder::visitArgTransformOne(NomParser::ArgTransformOneContext* context)
{
    auto* result = new AST::ANamedArgument(ConvertToken(context->getStart()));
    auto* firstList =
        new AST::AVector(ConvertToken(context->LPAREN()), ConvertToken(context->RPAREN()));
    for (auto* expr : context->expression())
        firstList->AddChild(visit(expr).as<AST::AExpr*>());
    result->AddChild(firstList);
    return result;
}

antlrcpp::Any CFileBuilder::visitArgColor(NomParser::ArgColorContext* context)
{
    auto* result = new AST::ANamedArgument(ConvertToken(context->getStart()));
    auto* colorList = new AST::AVector(ConvertToken(context->LPAREN()->getSymbol()),
                                       ConvertToken(context->RPAREN()->getSymbol()));
    for (auto* expr : context->expression())
    {
        colorList->AddChild(visit(expr).as<AST::AExpr*>());
    }
    result->AddChild(colorList);
    return result;
}

antlrcpp::Any CFileBuilder::visitIdList(NomParser::IdListContext *context)
{
    auto* list = new AST::AVector(ConvertToken(context->LPAREN()), ConvertToken(context->RPAREN()));
    for (auto* expr : context->identList)
        list->AddChild(visit(expr).as<AST::AExpr*>());
    return static_cast<AST::AExpr*>(list);
}

antlrcpp::Any CFileBuilder::visitCmdExprListOne(NomParser::CmdExprListOneContext* context)
{
    auto* cmd = new AST::ACommand(ConvertToken(context->open), ConvertToken(context->end));
    cmd->PushPositionalArgument(visit(context->name));
    auto* list = new AST::AVector(ConvertToken(context->LPAREN()), ConvertToken(context->RPAREN()));
    for (auto* expr : context->expression())
        list->AddChild(visit(expr).as<AST::AExpr*>());
    cmd->PushPositionalArgument(list);
    // Handle arguments other than name
    for (auto* arg : context->idList())
        cmd->PushPositionalArgument(visit(arg));
    return cmd;
}

antlrcpp::Any CFileBuilder::visitCmdIdListOne(NomParser::CmdIdListOneContext* context)
{
    auto* cmd = new AST::ACommand(ConvertToken(context->open), ConvertToken(context->end));
    cmd->PushPositionalArgument(visit(context->name));
    cmd->PushPositionalArgument(visit(context->idList()));
    // Handle arguments other than name
    for (auto* arg : context->argOrder())
        cmd->AddNamedArgument(visit(arg));
    for (auto* arg : context->argClosed())
        cmd->AddNamedArgument(visit(arg));
    for (auto* arg : context->argSlices())
        cmd->AddNamedArgument(visit(arg));
    for (auto* arg : context->argSurface())
        cmd->AddNamedArgument(visit(arg));
    return cmd;
}

antlrcpp::Any CFileBuilder::visitCmdSubCmds(NomParser::CmdSubCmdsContext* context)
{
    auto* cmd = new AST::ACommand(ConvertToken(context->open), ConvertToken(context->end));
    cmd->PushPositionalArgument(visit(context->name));
    for (auto* subCmd : context->command())
        cmd->AddSubCommand(visit(subCmd));
    return cmd;
}

antlrcpp::Any CFileBuilder::visitCmdInstance(NomParser::CmdInstanceContext* context)
{
    auto* cmd = new AST::ACommand(ConvertToken(context->open), ConvertToken(context->end));
    cmd->PushPositionalArgument(visit(context->name));
    cmd->PushPositionalArgument(visit(context->entity));
    for (auto* arg : context->argHidden())
        cmd->AddNamedArgument(visit(arg));
    for (auto* arg : context->argSurface())
        cmd->AddNamedArgument(visit(arg));
    for (auto* arg : context->argTransform())
        cmd->AddTransform(visit(arg));
    return cmd;
}

antlrcpp::Any CFileBuilder::visitCmdSurface(NomParser::CmdSurfaceContext* context)
{
    auto* cmd = new AST::ACommand(ConvertToken(context->open), ConvertToken(context->end));
    cmd->PushPositionalArgument(visit(context->name));
    cmd->AddNamedArgument(visit(context->argColor()));
    return cmd;
}

antlrcpp::Any CFileBuilder::visitCmdArgSurface(NomParser::CmdArgSurfaceContext* context)
{
    auto* cmd = new AST::ACommand(ConvertToken(context->open), ConvertToken(context->end));
    cmd->AddNamedArgument(visit(context->argSurface()));
    return cmd;
}

antlrcpp::Any CFileBuilder::visitCmdBank(NomParser::CmdBankContext* context)
{
    auto* cmd = new AST::ACommand(ConvertToken(context->open), ConvertToken(context->end));
    cmd->PushPositionalArgument(visit(context->name));
    for (auto* set : context->set())
        cmd->AddSubCommand(visit(set));
    return cmd;
}

antlrcpp::Any CFileBuilder::visitCmdDelete(NomParser::CmdDeleteContext* context)
{
    auto* cmd = new AST::ACommand(ConvertToken(context->open), ConvertToken(context->end));
    for (auto* deleteFace : context->deleteFace())
        cmd->AddSubCommand(visit(deleteFace));
    return cmd;
}

antlrcpp::Any CFileBuilder::visitCmdSubdivision(NomParser::CmdSubdivisionContext* context)
{
    auto* cmd = new AST::ACommand(ConvertToken(context->open), ConvertToken(context->end));
    cmd->PushPositionalArgument(visit(context->name));
    auto* namedArg = new AST::ANamedArgument(ConvertToken(context->k1));
    namedArg->AddChild(visit(context->v1).as<AST::AExpr*>());
    cmd->AddNamedArgument(namedArg);

    namedArg = new AST::ANamedArgument(ConvertToken(context->k2));
    namedArg->AddChild(visit(context->v2).as<AST::AExpr*>());
    cmd->AddNamedArgument(namedArg);
    return cmd;
}

antlrcpp::Any CFileBuilder::visitCmdOffset(NomParser::CmdOffsetContext* context)
{
    auto* cmd = new AST::ACommand(ConvertToken(context->open), ConvertToken(context->end));
    cmd->PushPositionalArgument(visit(context->name));
    auto* namedArg = new AST::ANamedArgument(ConvertToken(context->k1));
    namedArg->AddChild(visit(context->v1).as<AST::AExpr*>());
    cmd->AddNamedArgument(namedArg);

    namedArg = new AST::ANamedArgument(ConvertToken(context->k2));
    namedArg->AddChild(visit(context->v2).as<AST::AExpr*>());
    cmd->AddNamedArgument(namedArg);

    namedArg = new AST::ANamedArgument(ConvertToken(context->k3));
    namedArg->AddChild(visit(context->v3).as<AST::AExpr*>());
    cmd->AddNamedArgument(namedArg);

    namedArg = new AST::ANamedArgument(ConvertToken(context->k4));
    namedArg->AddChild(visit(context->v4).as<AST::AExpr*>());
    cmd->AddNamedArgument(namedArg);
    return cmd;
}

antlrcpp::Any CFileBuilder::visitSet(NomParser::SetContext* context)
{
    auto* cmd = new AST::ACommand(ConvertToken(context->open), nullptr);
    cmd->PushPositionalArgument(visit(context->ident()));
    for (auto* expr : context->expression())
        cmd->PushPositionalArgument(visit(expr));
    for (auto* time : context->argTime())
        cmd->AddNamedArgument(visit(time));
    for (auto* frame : context->argFrame())
        cmd->AddNamedArgument(visit(frame));
    return cmd;
}

antlrcpp::Any CFileBuilder::visitDeleteFace(NomParser::DeleteFaceContext* context)
{
    auto* cmd = new AST::ACommand(ConvertToken(context->open), ConvertToken(context->end));
    cmd->PushPositionalArgument(visit(context->ident()));
    return cmd;
}

antlrcpp::Any CFileBuilder::visitCall(NomParser::CallContext* context)
{
    auto* argList =
        new AST::AVector(ConvertToken(context->LPAREN()), ConvertToken(context->RPAREN()));
    argList->AddChild(visit(context->expression()).as<AST::AExpr*>());
    auto* funcToken = ConvertToken(context->ident()->IDENT());
    return static_cast<AST::AExpr*>(new AST::ACall(funcToken, argList));
}

antlrcpp::Any CFileBuilder::visitUnaryOp(NomParser::UnaryOpContext* context)
{
    if (context->PLUS())
        return static_cast<AST::AExpr*>(new AST::AUnaryOp(ConvertToken(context->PLUS()), visit(context->expression())));
    if (context->MINUS())
        return static_cast<AST::AExpr*>(new AST::AUnaryOp(ConvertToken(context->MINUS()), visit(context->expression())));
    throw AST::CSemanticError("Invalid unary operator", nullptr);
}

antlrcpp::Any CFileBuilder::visitSubExpParen(NomParser::SubExpParenContext* context)
{
    return static_cast<AST::AExpr*>(new AST::AWrappedExpr(ConvertToken(context->LPAREN()), ConvertToken(context->RPAREN()),
                                 nullptr, visit(context->expression())));
}

antlrcpp::Any CFileBuilder::visitSubExpCurly(NomParser::SubExpCurlyContext* context)
{
    return static_cast<AST::AExpr*>(new AST::AWrappedExpr(ConvertToken(context->beg), ConvertToken(context->end),
                                 ConvertToken(context->sec), visit(context->expression())));
}

antlrcpp::Any CFileBuilder::visitBinOp(NomParser::BinOpContext* context)
{

    auto left = visit(context->expression(0));
    auto right = visit(context->expression(1));
    auto* op = ConvertToken(context->op);
    return static_cast<AST::AExpr*>(new AST::ABinaryOp(op, left, right));
}

antlrcpp::Any CFileBuilder::visitScientific(NomParser::ScientificContext* context)
{
    return static_cast<AST::AExpr*>(new AST::ANumber(ConvertToken(context->SCIENTIFIC_NUMBER())));
}

antlrcpp::Any CFileBuilder::visitIdent(NomParser::IdentContext* context)
{
    return static_cast<AST::AExpr*>(new AST::AIdent(ConvertToken(context->IDENT())));
}

antlrcpp::Any CFileBuilder::visitAtomExpr(NomParser::AtomExprContext* context)
{
    return visit(context->atom());
}

AST::CToken* CFileBuilder::ConvertToken(antlr4::Token* token)
{
    auto start = token->getStartIndex();
    auto len = token->getStopIndex() - start + 1;
    return new AST::CToken(token->getText(), 0, start);
}

AST::CToken* CFileBuilder::ConvertToken(antlr4::tree::TerminalNode* token)
{
    auto start = token->getSymbol()->getStartIndex();
    auto len = token->getSymbol()->getStopIndex() - start + 1;
    return new AST::CToken(token->getText(), 0, start);
}

}
