#pragma once
#include "DataTypeHelper.h"
#include "ConvertToHLSLType.h"
#include <string>
#include <utility>

namespace Nome
{

class IRVisitor;

//TODO: use a pool allocator, for now, we never delete those objects!

class IRNode
{
public:
	virtual ~IRNode() = default;
	virtual void Accept(IRVisitor& v);
};

//Beginning of expressions

class IRExpr : public IRNode
{
public:
	void Accept(IRVisitor& v) override;
	EDataType DataType = EDataType::Invalid;
};

template <typename T>
class IRInputAttr : public IRExpr
{
public:
	IRInputAttr(std::string name) : Name(std::move(name))
	{
		DataType = ToDataType<T>::Result;
	}

	void Accept(IRVisitor& v) override;

    std::string Name;
};

typedef IRInputAttr<float> IRInputAttrFloat;
typedef IRInputAttr<Vector3> IRInputAttrVec3;
typedef IRInputAttr<Vector4> IRInputAttrVec4;

class IRRef : public IRExpr
{
public:
	enum class ERefDomain
	{
		Vertex
	};

	IRRef(std::string name, ERefDomain domain = ERefDomain::Vertex) : Name(std::move(name)), Domain(domain) {}

	void Accept(IRVisitor& v) override;
	
	std::string Name;
	ERefDomain Domain;
};

template <typename T>
class IRConstant : public IRExpr
{
public:
	IRConstant(T value) : Value(std::move(value))
	{
		DataType = ToDataType<T>::Result;
	}

	void Accept(IRVisitor& v) override;
	
	T Value;
};

typedef IRConstant<float> IRConstantFloat;
typedef IRConstant<Vector3> IRConstantVec3;
typedef IRConstant<Vector4> IRConstantVec4;
typedef IRConstant<Matrix3> IRConstantMat3;
typedef IRConstant<Matrix4> IRConstantMat4;

class IRUnaryOp : public IRExpr
{
public:
	enum EOp
	{
		Invalid,
		Neg,
		Sqrt,
		Sin,
		Cos
	};

	IRUnaryOp(EOp op, IRExpr* right) : Op(op), Right(right) {}

	void Accept(IRVisitor& v) override;

	EOp Op;
	IRExpr* Right;
};

class IRBinaryOp : public IRExpr
{
public:
	IRBinaryOp(IRExpr* left, IRExpr* right) : Left(left), Right(right) {}

	void Accept(IRVisitor& v) override;
	
	IRExpr* Left;
	IRExpr* Right;
};

class IRAdd : public IRBinaryOp
{
public:
	using IRBinaryOp::IRBinaryOp;

	void Accept(IRVisitor& v) override;
};

class IRSub : public IRBinaryOp
{
public:
	using IRBinaryOp::IRBinaryOp;

	void Accept(IRVisitor& v) override;
};

class IRMul : public IRBinaryOp
{
public:
	using IRBinaryOp::IRBinaryOp;

	void Accept(IRVisitor& v) override;
};

class IRDiv : public IRBinaryOp
{
public:
	using IRBinaryOp::IRBinaryOp;

	void Accept(IRVisitor& v) override;
};

class IRDotProduct : public IRBinaryOp
{
public:
	using IRBinaryOp::IRBinaryOp;

	void Accept(IRVisitor& v) override;
};

class IRCrossProduct : public IRBinaryOp
{
public:
	using IRBinaryOp::IRBinaryOp;

	void Accept(IRVisitor& v) override;
};

class IRConstructMat3 : public IRExpr
{
public:
	IRConstructMat3(IRExpr* m00, IRExpr* m01, IRExpr* m02,
		IRExpr* m10, IRExpr* m11, IRExpr* m12,
		IRExpr* m20, IRExpr* m21, IRExpr* m22)
	{
		Args[0] = m00; Args[1] = m01; Args[2] = m02;
		Args[3] = m10; Args[4] = m11; Args[5] = m12;
		Args[6] = m20; Args[7] = m21; Args[8] = m22;
	}

	void Accept(IRVisitor& v) override;

	IRExpr* Args[9];
};

//Beginning of statements

class IRStmt : public IRNode
{
public:
	void Accept(IRVisitor& v) override;

	IRStmt* Prev = nullptr;
	IRStmt* Next = nullptr;
};

class IRAssign : public IRStmt
{
public:
	IRAssign(IRRef* target, IRExpr* rvalue) : Target(target), RValue(rvalue) {}

	void Accept(IRVisitor& v) override;

	IRRef* Target;
	IRExpr* RValue;
};

class IRMaterializeAttr : public IRStmt
{
public:
	IRMaterializeAttr(IRRef* target) : Target(target) {}

	void Accept(IRVisitor& v) override;

	IRRef* Target;
};

class IROffset : public IRStmt
{
public:
	IROffset(IRRef* dir) : Dir(dir) {}

	void Accept(IRVisitor& v) override;

	IRRef* Dir;
};

class IRSubdivideCC : public IRStmt
{
public:
	void Accept(IRVisitor& v) override;
};

// Package
class IRProgram
{
public:
	IRProgram(IRStmt* initialStatment) : InitStmt(initialStatment) {}

	IRStmt* InitStmt;
};

} /* namespace Nome */
