#include "IR.h"
#include "IRVisitor.h"

namespace Nome
{

void IRNode::Accept(IRVisitor& v) { throw CIRException("Plain IRNode should not be used."); }
void IRExpr::Accept(IRVisitor& v) { v.Visit(this); }
void IRInputAttrFloat::Accept(IRVisitor& v) { v.Visit(this); }
void IRInputAttrVec3::Accept(IRVisitor& v) { v.Visit(this); }
void IRInputAttrVec4::Accept(IRVisitor& v) { v.Visit(this); }
void IRInputAttrMat3::Accept(IRVisitor& v) { v.Visit(this); }
void IRInputAttrMat4::Accept(IRVisitor& v) { v.Visit(this); }
void IRRef::Accept(IRVisitor& v) { v.Visit(this); }
void IRConstantFloat::Accept(IRVisitor& v) { v.Visit(this); }
void IRConstantVec3::Accept(IRVisitor& v) { v.Visit(this); }
void IRConstantVec4::Accept(IRVisitor& v) { v.Visit(this); }
void IRConstantMat3::Accept(IRVisitor& v) { v.Visit(this); }
void IRConstantMat4::Accept(IRVisitor& v) { v.Visit(this); }
void IRBinaryOp::Accept(IRVisitor& v) { v.Visit(this); }
void IRAdd::Accept(IRVisitor& v) { v.Visit(this); }
void IRMul::Accept(IRVisitor& v) { v.Visit(this); }
void IRStmt::Accept(IRVisitor& v) { v.Visit(this); }
void IRAssign::Accept(IRVisitor& v) { v.Visit(this); }
void IRMaterializeAttr::Accept(IRVisitor& v) { v.Visit(this); }
void IROffset::Accept(IRVisitor& v) { v.Visit(this); }
void IRSubdivideCC::Accept(IRVisitor& v) { v.Visit(this); }

} /* namespace Nome */
