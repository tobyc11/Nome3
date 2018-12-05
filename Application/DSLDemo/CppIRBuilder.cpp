#include "CppIRBuilder.h"

namespace Nome::CppIRBuilder
{

std::stack<BuilderContext*> BuilderContextStack;

BuilderContext& BuilderContext::Top()
{
	return *BuilderContextStack.top();
}

BuilderContext::BuilderContext()
{
	BuilderContextStack.push(this);
}

BuilderContext::~BuilderContext()
{
	BuilderContextStack.pop();
}

void BuilderContext::AddStatment(IRStmt* statment)
{
	if (!LastStmt)
	{
		LastStmt = statment;
		return;
	}

	LastStmt->Next = statment;
	statment->Prev = LastStmt;
	LastStmt = statment;
}

IRProgram* BuilderContext::GetProgram()
{
	IRStmt* curr = LastStmt;
	while (curr->Prev)
		curr = curr->Prev;
	return new IRProgram(curr);
}

Expr operator*(const Expr& left, const Expr& right)
{
	Expr ret;
	ret.ExprPtr = new IRMul(left.ExprPtr, right.ExprPtr);
	return ret;
}

} /* namespace Nome */
