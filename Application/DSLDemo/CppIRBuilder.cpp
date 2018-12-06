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

Expr Expr::operator-()
{
	Expr ret;
	ret.ExprPtr = new IRUnaryOp(IRUnaryOp::EOp::Neg, this->ExprPtr);
	return ret;
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

Expr operator/(const Expr& left, const Expr& right)
{
	Expr ret;
	ret.ExprPtr = new IRDiv(left.ExprPtr, right.ExprPtr);
	return ret;
}

Expr Sqrt(const Expr& operand)
{
	Expr ret;
	ret.ExprPtr = new IRUnaryOp(IRUnaryOp::EOp::Sqrt, operand.ExprPtr);
	return ret;
}

Expr Sin(const Expr& operand)
{
	Expr ret;
	ret.ExprPtr = new IRUnaryOp(IRUnaryOp::EOp::Sin, operand.ExprPtr);
	return ret;
}

Expr Cos(const Expr& operand)
{
	Expr ret;
	ret.ExprPtr = new IRUnaryOp(IRUnaryOp::EOp::Cos, operand.ExprPtr);
	return ret;
}

Expr Dot(const Expr& left, const Expr& right)
{
	Expr ret;
	ret.ExprPtr = new IRDotProduct(left.ExprPtr, right.ExprPtr);
	return ret;
}

Expr Cross(const Expr& left, const Expr& right)
{
	Expr ret;
	ret.ExprPtr = new IRCrossProduct(left.ExprPtr, right.ExprPtr);
	return ret;
}

Expr Mat3(const Expr& m00, const Expr& m01, const Expr& m02,
	const Expr& m10, const Expr& m11, const Expr& m12,
	const Expr& m20, const Expr& m21, const Expr& m22)
{
	Expr ret;
	ret.ExprPtr = new IRConstructMat3(m00.ExprPtr, m01.ExprPtr, m02.ExprPtr,
		m10.ExprPtr, m11.ExprPtr, m12.ExprPtr,
		m20.ExprPtr, m21.ExprPtr, m22.ExprPtr);
	return ret;
}

} /* namespace Nome */
