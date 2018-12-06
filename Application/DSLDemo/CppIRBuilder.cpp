#include "CppIRBuilder.h"

namespace Nome::CppIRBuilder
{

std::stack<ScopedBuilderContext*> ScopedBuilderContextStack;

ScopedBuilderContext& ScopedBuilderContext::Top()
{
	return *ScopedBuilderContextStack.top();
}

ScopedBuilderContext::ScopedBuilderContext()
{
	ScopedBuilderContextStack.push(this);
}

ScopedBuilderContext::~ScopedBuilderContext()
{
	ScopedBuilderContextStack.pop();
}

void ScopedBuilderContext::AddStatment(IRStmt* statment)
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

IRProgram* ScopedBuilderContext::GetProgram()
{
	IRStmt* curr = LastStmt;
	while (curr->Prev)
		curr = curr->Prev;
	return new IRProgram(curr);
}

Expr operator+(const Expr& left, const Expr& right)
{
	Expr ret;
	ret.ExprPtr = new IRAdd(left.ExprPtr, right.ExprPtr);
	return ret;
}

Expr operator*(const Expr& left, const Expr& right)
{
	Expr ret;
	ret.ExprPtr = new IRMul(left.ExprPtr, right.ExprPtr);
	return ret;
}

} /* namespace Nome */
