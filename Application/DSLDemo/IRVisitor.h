#pragma once
#include "IR.h"

namespace Nome
{

class CIRException : public std::runtime_error
{
public:
	using runtime_error::runtime_error;
};

class IRVisitor
{
public:
    //virtual void Visit(IRNode* node) {} not called by IRNode, throws CIRException instead
	virtual void Visit(IRExpr* node) {}
	virtual void Visit(IRInputAttrFloat* node) {}
	virtual void Visit(IRInputAttrVec3* node) {}
	virtual void Visit(IRInputAttrVec4* node) {}
	virtual void Visit(IRRef* node) {}
	virtual void Visit(IRConstantFloat* node) {}
	virtual void Visit(IRConstantVec3* node) {}
	virtual void Visit(IRConstantVec4* node) {}
	virtual void Visit(IRConstantMat3* node) {}
	virtual void Visit(IRConstantMat4* node) {}
	virtual void Visit(IRUnaryOp* node) {}
	virtual void Visit(IRBinaryOp* node) {}
	virtual void Visit(IRAdd* node) {}
	virtual void Visit(IRSub* node) {}
	virtual void Visit(IRMul* node) {}
	virtual void Visit(IRDiv* node) {}
	virtual void Visit(IRDotProduct* node) {}
	virtual void Visit(IRCrossProduct* node) {}
	virtual void Visit(IRConstructMat3* node) {}
	virtual void Visit(IRStmt* node) {}
	virtual void Visit(IRAssign* node) {}
	virtual void Visit(IRMaterializeAttr* node) {}
	virtual void Visit(IROffset* node) {}
	virtual void Visit(IRSubdivideCC* node) {}
	virtual void Visit(IRSubdivideAdHoc* node) {}
};

} /* namespace Nome */
