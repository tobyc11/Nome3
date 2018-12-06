#pragma once
#include "IR.h"

#include <stack>

namespace Nome::CppIRBuilder
{

class ScopedBuilderContext
{
public:
	static ScopedBuilderContext& Top();

	ScopedBuilderContext();
	~ScopedBuilderContext();

	void AddStatment(IRStmt* statment);

	IRProgram* GetProgram();

private:
	IRStmt* LastStmt = nullptr;
};

//Stores all the builders currently on the stack
extern std::stack<ScopedBuilderContext*> ScopedBuilderContextStack;

class Expr
{
public:
	IRExpr* ExprPtr;
};

Expr operator+(const Expr& left, const Expr& right);

template <typename T>
Expr operator+(const T& left, const Expr& right)
{
	Expr ret;
	IRExpr* constExpr = new IRConstant<T>(left);
	ret.ExprPtr = new IRAdd(constExpr, right.ExprPtr);
	return ret;
}

template <typename T>
Expr operator+(const Expr& left, const T& right)
{
	Expr ret;
	IRExpr* constExpr = new IRConstant<T>(right);
	ret.ExprPtr = new IRAdd(left.ExprPtr, constExpr);
	return ret;
}

Expr operator*(const Expr& left, const Expr& right);

template <typename T>
Expr operator*(const T& left, const Expr& right)
{
	Expr ret;
	IRExpr* constExpr = new IRConstant<T>(left);
	ret.ExprPtr = new IRMul(constExpr, right.ExprPtr);
	return ret;
}

template <typename T>
Expr operator*(const Expr& left, const T& right)
{
	Expr ret;
	IRExpr* constExpr = new IRConstant<T>(right);
	ret.ExprPtr = new IRMul(left.ExprPtr, constExpr);
	return ret;
}

template <typename T>
class InputAttr : public Expr
{
public:
	InputAttr(std::string name)
	{
		ExprPtr = new IRInputAttr<T>(name);
	}
};

class VertAttrRef : public Expr
{
public:
	VertAttrRef(std::string name)
	{
		ExprPtr = Ref = new IRRef(name, IRRef::ERefDomain::Vertex);
	}

	void operator=(const Expr& rhs)
	{
		auto* statment = new IRAssign(Ref, rhs.ExprPtr);
		ScopedBuilderContext::Top().AddStatment(statment);
	}

private:
	IRRef* Ref;
};

typedef VertAttrRef Attr;

class MaterializeAttr
{
public:
	MaterializeAttr(const std::string& name)
	{
		IRStmt* stmt = new IRMaterializeAttr(new IRRef(name));
		ScopedBuilderContext::Top().AddStatment(stmt);
	}
};

class Offset
{
public:
	Offset(const std::string& dir)
	{
		IRStmt* stmt = new IROffset(new IRRef(dir));
		ScopedBuilderContext::Top().AddStatment(stmt);
	}
};

class SubdivideCatmullClark
{
public:
	SubdivideCatmullClark()
	{
		IRStmt* stmt = new IRSubdivideCC();
		ScopedBuilderContext::Top().AddStatment(stmt);
	}
};

} /* namespace Nome */
