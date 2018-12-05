#include "EffiCompiler.h"
#include "MOMaterializeAttr.h"
#include <StringPrintf.h>
#include <stdexcept>

namespace Nome
{

class CEffiCompileError : public CIRException
{
public:
	using CIRException::CIRException;
};

std::string CHLSLCodeGen::CodeGen(IRExpr* node, const std::string& fnName)
{
	node->Accept(*this);
	return tc::StringPrintf("%s %s(VSIn input)\n{\n"
	"\treturn %s;\n"
	"}\n", ConvertToHLSLType(node->DataType).c_str(), fnName.c_str(), RetVal.c_str());
}

void CHLSLCodeGen::Visit(IRInputAttrVec3* node)
{
	RetVal = tc::StringPrintf("input.%s", node->Name.c_str());

	unsigned int attribIndex = (unsigned int)ReferredFields.size();
	ReferredFields.insert({ node->Name, {node->DataType, attribIndex} });
}

void CHLSLCodeGen::Visit(IRInputAttrVec4* node)
{
	RetVal = tc::StringPrintf("input.%s", node->Name.c_str());

	unsigned int attribIndex = (unsigned int)ReferredFields.size();
	ReferredFields.insert({ node->Name, {node->DataType, attribIndex} });
}

void CHLSLCodeGen::Visit(IRInputAttrMat3* node)
{
	RetVal = tc::StringPrintf("input.%s", node->Name.c_str());

	unsigned int attribIndex = (unsigned int)ReferredFields.size();
	ReferredFields.insert({ node->Name, {node->DataType, attribIndex} });
}

void CHLSLCodeGen::Visit(IRInputAttrMat4* node)
{
	RetVal = tc::StringPrintf("input.%s", node->Name.c_str());

	unsigned int attribIndex = (unsigned int)ReferredFields.size();
	ReferredFields.insert({ node->Name, {node->DataType, attribIndex} });
}

void CHLSLCodeGen::Visit(IRConstantFloat* node)
{
	RetVal = tc::StringPrintf("%f", node->Value);
}

void CHLSLCodeGen::Visit(IRConstantVec3* node)
{
	RetVal = tc::StringPrintf("float3(%f, %f, %f)", node->Value.x, node->Value.y, node->Value.z);
}

void CHLSLCodeGen::Visit(IRConstantVec4* node)
{
	RetVal = tc::StringPrintf("float4(%f, %f, %f, %f)", node->Value.x, node->Value.y, node->Value.z, node->Value.w);
}

void CHLSLCodeGen::Visit(IRConstantMat3* node)
{
	RetVal = tc::StringPrintf("float3x3(%f, %f, %f, %f, %f, %f, %f, %f, %f)",
		node->Value[0][0], node->Value[0][1], node->Value[0][2],
		node->Value[1][0], node->Value[1][1], node->Value[1][2], 
		node->Value[2][0], node->Value[2][1], node->Value[2][2]);
}

void CHLSLCodeGen::Visit(IRConstantMat4* node)
{
	RetVal = tc::StringPrintf("float4x4(%f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f)",
		node->Value[0][0], node->Value[0][1], node->Value[0][2], node->Value[0][3],
		node->Value[1][0], node->Value[1][1], node->Value[1][2], node->Value[1][3],
		node->Value[2][0], node->Value[2][1], node->Value[2][2], node->Value[2][3],
		node->Value[3][0], node->Value[3][1], node->Value[3][2], node->Value[3][3]);
}

void CHLSLCodeGen::Visit(IRAdd* node)
{
	node->Left->Accept(*this);
	std::string left = "(" + RetVal + ")";
	node->Right->Accept(*this);
	std::string right = "(" + RetVal + ")";
	RetVal = left + " + " + right;
}

void CHLSLCodeGen::Visit(IRMul* node)
{
	node->Left->Accept(*this);
	std::string left = "(" + RetVal + ")";
	node->Right->Accept(*this);
	std::string right = "(" + RetVal + ")";
	RetVal = "mul(" + left + ", " + right + ")";
}

CHLSLInputStructGen::CHLSLInputStructGen(const std::unordered_map<std::string, std::pair<EDataType, unsigned int>>& fields)
{
	std::string declarations;
	for (const auto& field : fields)
	{
		std::string hlslType = ConvertToHLSLType(field.second.first);
		declarations += tc::StringPrintf("\t%s %s : ATTRIBUTE%d;\n", hlslType.c_str(), field.first.c_str(), field.second.second);
	}
	Result = tc::StringPrintf("struct VSIn\n"
	"{\n"
	"%s"
	"};\n", declarations.c_str());
}

CHLSLStructGen::CHLSLStructGen(const std::string& structName, 
	const std::unordered_map<std::string, std::pair<EDataType, std::string>>& fields)
{
	std::string declarations;
	for (const auto& field : fields)
	{
		std::string hlslType = ConvertToHLSLType(field.second.first);
		declarations += tc::StringPrintf("\t%s %s : %s;\n", hlslType.c_str(), field.first.c_str(), field.second.second.c_str());
	}
	Result = tc::StringPrintf("struct %s\n"
		"{\n"
		"%s"
		"};\n", structName.c_str(), declarations.c_str());
}

void CEffiCompiler::Compile(IRProgram* program)
{
	CompiledPipeline = new CEffiCompiledPipeline();
	try
	{
		IRStmt* curr = program->InitStmt;
		while (curr)
		{
			curr->Accept(*this);
			curr = curr->Next;
		}
	}
	catch (const CEffiCompileError& e)
	{
		printf("ERROR during compilation: %s\n", e.what());
		return;
	}
}

void CEffiCompiler::GenericVisit(IRNode* node)
{
	node->Accept(*this);
}

void CEffiCompiler::Visit(IRExpr* node)
{
	throw CEffiCompileError("Unexpected IR node encountered.");
}

void CEffiCompiler::Visit(IRInputAttrVec3* node)
{
	RetExpr = node;
}

void CEffiCompiler::Visit(IRInputAttrVec4* node)
{
	RetExpr = node;
}

void CEffiCompiler::Visit(IRInputAttrMat3* node)
{
	RetExpr = node;
}

void CEffiCompiler::Visit(IRInputAttrMat4* node)
{
	RetExpr = node;
}

void CEffiCompiler::Visit(IRRef* node)
{
	auto iter = VSymTab.find(node->Name);
	if (iter == VSymTab.end())
		throw CEffiCompileError(tc::StringPrintf("Reference to %s not found.", node->Name));

	RetExpr = iter->second;
}

void CEffiCompiler::Visit(IRConstantFloat* node)
{
	RetExpr = node;
}

void CEffiCompiler::Visit(IRConstantVec3* node)
{
	RetExpr = node;
}

void CEffiCompiler::Visit(IRConstantVec4* node)
{
	RetExpr = node;
}

void CEffiCompiler::Visit(IRConstantMat3* node)
{
	RetExpr = node;
}

void CEffiCompiler::Visit(IRConstantMat4* node)
{
	RetExpr = node;
}

void CEffiCompiler::Visit(IRBinaryOp* node)
{
	throw CEffiCompileError("Unexpected IR node encountered.");
}

void CEffiCompiler::Visit(IRAdd* node)
{
	//Recurse down
	GenericVisit(node->Left);
	node->Left = RetExpr;
	GenericVisit(node->Right);
	node->Right = RetExpr;

	//Type check, and return
	RetExpr = node;
	if (node->Left->DataType == node->Right->DataType)
	{
		node->DataType = node->Left->DataType;
		return;
	}
	throw CEffiCompileError("Invalid add, type mismatch.");
}

void CEffiCompiler::Visit(IRMul* node)
{
	//Recurse down
	GenericVisit(node->Left);
	node->Left = RetExpr;
	GenericVisit(node->Right);
	node->Right = RetExpr;

	//Type check, and return
	RetExpr = node;
	if (node->Left->DataType == node->Right->DataType)
	{
		node->DataType = node->Left->DataType;
		return;
	}
	if (node->Left->DataType == EDataType::Float3x3 && node->Right->DataType == EDataType::Float3)
	{
		node->DataType = node->Right->DataType;
		return;
	}
	if (node->Left->DataType == EDataType::Float4x4 && node->Right->DataType == EDataType::Float4)
	{
		node->DataType = node->Right->DataType;
		return;
	}
	throw CEffiCompileError("Invalid mul, type mismatch.");
}

void CEffiCompiler::Visit(IRStmt* node)
{
	throw CEffiCompileError("Unexpected IR node encountered.");
}

void CEffiCompiler::Visit(IRAssign* node)
{
	GenericVisit(node->RValue);
	VSymTab[node->Target->Name] = RetExpr;
}

void CEffiCompiler::Visit(IRMaterializeAttr* node)
{
	auto iter = VSymTab.find(node->Target->Name);
	if (iter == VSymTab.end())
		throw CEffiCompileError(tc::StringPrintf("Cannot materialize %s, reference not found.", node->Target->Name));
	
	//TODO: do stuff
	//iter->second is the attribute to materialize
	CHLSLCodeGen codegen;
	auto shaderFunc = codegen.CodeGen(iter->second, "MaterializeAttr");

	CHLSLInputStructGen vsinGen{ codegen.ReferredFields };
	auto shaderVSIn = vsinGen.Result;

	std::unordered_map<std::string, std::pair<EDataType, std::string>> outVars;
	outVars.insert({ "Pos", {iter->second->DataType, "SV_Position"} });
	CHLSLStructGen vsOutGen{ "VSOut", outVars };
	auto shaderVSOut = vsOutGen.Result;

	MOMaterializeAttr* meshOp = new MOMaterializeAttr(EffiContext->GetGraphicsDevice(), shaderFunc, shaderVSIn, shaderVSOut);
	CompiledPipeline->AddOperator(meshOp);

	switch (iter->second->DataType)
	{
	case EDataType::Float:
		VSymTab[node->Target->Name] = new IRInputAttr<float>(node->Target->Name);
		break;
	case EDataType::Float3:
		VSymTab[node->Target->Name] = new IRInputAttr<Vector3>(node->Target->Name);
		break;
	case EDataType::Float4:
		VSymTab[node->Target->Name] = new IRInputAttr<Vector4>(node->Target->Name);
		break;
	case EDataType::Float3x3:
		VSymTab[node->Target->Name] = new IRInputAttr<Matrix3>(node->Target->Name);
		break;
	case EDataType::Float4x4:
		VSymTab[node->Target->Name] = new IRInputAttr<Matrix4>(node->Target->Name);
		break;
	case EDataType::Invalid:
	default:
		throw CEffiCompileError(tc::StringPrintf("Attribute %s cannot be materialized due to invalid type.",
			node->Target->Name.c_str()));
		break;
	}
}

void CEffiCompiler::Visit(IROffset* node)
{
}

void CEffiCompiler::Visit(IRSubdivideCC* node)
{
}

} /* namespace Nome */
