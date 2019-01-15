#pragma once
#include "AST.h"

namespace Nome
{

template <typename T>
class TASTConsumer
{
public:
    TASTConsumer(CASTContext& ast) : AST(ast)
    {
    }

    void Traverse()
    {
        const auto& cmds = AST.GetCommands();
		for (ACommand* cmd : cmds)
		{
			RecurseCmd(cmd);
		}
    }

private:
	void RecurseCmd(ACommand* cmd)
	{
		static_cast<T*>(this)->BeginCommand(cmd);
		for (ACommand* subCmd : cmd->SubCommands)
			RecurseCmd(subCmd);
		static_cast<T*>(this)->EndCommand(cmd);
	}

protected:
    CASTContext& AST;
};

template <typename T>
class TExprVisitor
{
public:
	void Visit(AExpr* expr)
	{
		if (auto* node = ast_as<AIdent*>(expr))
			static_cast<T*>(this)->VisitIdent(node);
		else if (auto* node = ast_as<ANumber*>(expr))
			static_cast<T*>(this)->VisitNumber(node);
		else if (auto* node = ast_as<AUnaryOp*>(expr))
			static_cast<T*>(this)->VisitUnaryOp(node);
		else if (auto* node = ast_as<ABinaryOp*>(expr))
			static_cast<T*>(this)->VisitBinaryOp(node);
	}

	void VisitIdent(AIdent* ident) {}
	void VisitNumber(ANumber* number) {}
	void VisitUnaryOp(AUnaryOp* op) {}
	void VisitBinaryOp(ABinaryOp* op) {}
};

}
