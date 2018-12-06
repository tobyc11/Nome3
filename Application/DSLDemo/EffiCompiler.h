#pragma once
#include "Effi.h"
#include "IRVisitor.h"
#include "EffiCompiledPipeline.h"

#include <stdexcept>
#include <sstream>
#include <unordered_map>
#include <utility>

namespace Nome
{

class CEffiCompileError : public CIRException
{
public:
	using CIRException::CIRException;
};

class CHLSLCodeGen : public IRVisitor
{
public:
	std::string CodeGen(IRExpr* node, const std::string& fnName);

	//Exprs
	void Visit(IRInputAttrFloat* node) override;
	void Visit(IRInputAttrVec3* node) override;
	void Visit(IRInputAttrVec4* node) override;
	void Visit(IRRef* node) override;
	void Visit(IRConstantFloat* node) override;
	void Visit(IRConstantVec3* node) override;
	void Visit(IRConstantVec4* node) override;
	void Visit(IRConstantMat3* node) override;
	void Visit(IRConstantMat4* node) override;
	void Visit(IRUnaryOp* node) override;
	void Visit(IRAdd* node) override;
	void Visit(IRSub* node) override;
	void Visit(IRMul* node) override;
	void Visit(IRDiv* node) override;
	void Visit(IRDotProduct* node) override;
	void Visit(IRCrossProduct* node) override;
	void Visit(IRConstructMat3* node) override;

	std::unordered_map<std::string, std::pair<EDataType, unsigned int>> ReferredFields;

private:
	std::string NextLocal(IRNode* node);
	std::string NodeToLocalVar(IRNode* node);
	bool IsLocalAlready(IRNode* node);

	std::unordered_map<IRNode*, int> NodeToLocal;
	std::stringstream CodeStream;
};

class CHLSLInputStructGen
{
public:
	CHLSLInputStructGen(const std::unordered_map<std::string, std::pair<EDataType, unsigned int>>& fields);

	std::string Result;
};

class CHLSLStructGen
{
public:
	CHLSLStructGen(const std::string& structName,
		const std::unordered_map<std::string, std::pair<EDataType, std::string>>& fields);

	std::string Result;
};

class CEffiCompiler : public IRVisitor
{
public:
	CEffiCompiler(CEffiContext* context) : EffiContext(context)
	{
	}

	CEffiCompiledPipeline* Compile(IRProgram* program);

	void GenericVisit(IRNode* node);

	//Exprs
	void Visit(IRExpr* node) override;
	void Visit(IRInputAttrFloat* node) override;
	void Visit(IRInputAttrVec3* node) override;
	void Visit(IRInputAttrVec4* node) override;
	void Visit(IRRef* node) override;
	void Visit(IRConstantFloat* node) override;
	void Visit(IRConstantVec3* node) override;
	void Visit(IRConstantVec4* node) override;
	void Visit(IRConstantMat3* node) override;
	void Visit(IRConstantMat4* node) override;
	void Visit(IRUnaryOp* node) override;
	void Visit(IRBinaryOp* node) override;
	void Visit(IRAdd* node) override;
	void Visit(IRSub* node) override;
	void Visit(IRMul* node) override;
	void Visit(IRDiv* node) override;
	void Visit(IRDotProduct* node) override;
	void Visit(IRCrossProduct* node) override;
	void Visit(IRConstructMat3* node) override;

	//Statments
	void Visit(IRStmt* node) override;
	void Visit(IRAssign* node) override;
	void Visit(IRMaterializeAttr* node) override;
	void Visit(IROffset* node) override;
	void Visit(IRSubdivideCC* node) override;
	void Visit(IRSubdivideAdHoc* node) override;

private:
	//Symbol table for vertix attributes
	std::unordered_map<std::string, IRExpr*> VSymTab;

	//Used to store the return value during expression evaluation
	IRExpr* RetExpr = nullptr;

	CEffiCompiledPipeline* CompiledPipeline = nullptr;

	CEffiContext* EffiContext;
};

} /* namespace Nome */
