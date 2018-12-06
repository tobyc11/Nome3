#include "EffiCompiler.h"
#include "MOMaterializeAttr.h"
#include <StringPrintf.h>

namespace Nome
{

std::string CHLSLCodeGen::CodeGen(IRExpr* node, const std::string& fnName)
{
	node->Accept(*this);
	return tc::StringPrintf("%s %s(VSIn input)\n{\n"
	"%s\n"
	"\treturn %s;\n"
	"}\n", ConvertToHLSLType(node->DataType).c_str(), fnName.c_str(),
		CodeStream.str().c_str(),
		("local" + std::to_string(NodeToLocal.size() - 1)).c_str());
}

void CHLSLCodeGen::Visit(IRInputAttrFloat* node)
{
	if (IsLocalAlready(node))
		return;

	CodeStream << "\t" << ConvertToHLSLType(node->DataType) << " " << NextLocal(node) << " = " << "input." << node->Name << ";" << std::endl;

	unsigned int attribIndex = (unsigned int)ReferredFields.size();
	ReferredFields.insert({ node->Name, {node->DataType, attribIndex} });
}

void CHLSLCodeGen::Visit(IRInputAttrVec3* node)
{
	if (IsLocalAlready(node))
		return;

	CodeStream << "\t" << ConvertToHLSLType(node->DataType) << " " << NextLocal(node) << " = " << "input." << node->Name << ";" << std::endl;

	unsigned int attribIndex = (unsigned int)ReferredFields.size();
	ReferredFields.insert({ node->Name, {node->DataType, attribIndex} });
}

void CHLSLCodeGen::Visit(IRInputAttrVec4* node)
{
	if (IsLocalAlready(node))
		return;

	CodeStream << "\t" << ConvertToHLSLType(node->DataType) << " " << NextLocal(node) << " = " << "input." << node->Name << ";" << std::endl;

	unsigned int attribIndex = (unsigned int)ReferredFields.size();
	ReferredFields.insert({ node->Name, {node->DataType, attribIndex} });
}

void CHLSLCodeGen::Visit(IRRef* node)
{
	throw CEffiCompileError("Encountered: WTF IRRef?");
}

void CHLSLCodeGen::Visit(IRConstantFloat* node)
{
	if (IsLocalAlready(node))
		return;

	CodeStream << "\t" << ConvertToHLSLType(node->DataType) << " " << NextLocal(node) << " = " << node->Value << ";" << std::endl;
}

void CHLSLCodeGen::Visit(IRConstantVec3* node)
{
	if (IsLocalAlready(node))
		return;

	CodeStream << "\t" << ConvertToHLSLType(node->DataType) << " " << NextLocal(node) << " = " <<
		tc::StringPrintf("float3(%f, %f, %f)", node->Value.x, node->Value.y, node->Value.z) << ";" << std::endl;
}

void CHLSLCodeGen::Visit(IRConstantVec4* node)
{
	if (IsLocalAlready(node))
		return;

	CodeStream << "\t" << ConvertToHLSLType(node->DataType) << " " << NextLocal(node) << " = " <<
		tc::StringPrintf("float4(%f, %f, %f, %f)", node->Value.x, node->Value.y, node->Value.z, node->Value.w) << ";" << std::endl;
}

void CHLSLCodeGen::Visit(IRConstantMat3* node)
{
	if (IsLocalAlready(node))
		return;

	auto valStr = tc::StringPrintf("float3x3(%f, %f, %f, %f, %f, %f, %f, %f, %f)",
		node->Value[0][0], node->Value[0][1], node->Value[0][2],
		node->Value[1][0], node->Value[1][1], node->Value[1][2],
		node->Value[2][0], node->Value[2][1], node->Value[2][2]);

	CodeStream << "\t" << ConvertToHLSLType(node->DataType) << " " << NextLocal(node) << " = " << valStr << ";" << std::endl;
}

void CHLSLCodeGen::Visit(IRConstantMat4* node)
{
	if (IsLocalAlready(node))
		return;

	auto valStr = tc::StringPrintf("float4x4(%f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f)",
		node->Value[0][0], node->Value[0][1], node->Value[0][2], node->Value[0][3],
		node->Value[1][0], node->Value[1][1], node->Value[1][2], node->Value[1][3],
		node->Value[2][0], node->Value[2][1], node->Value[2][2], node->Value[2][3],
		node->Value[3][0], node->Value[3][1], node->Value[3][2], node->Value[3][3]);

	CodeStream << "\t" << ConvertToHLSLType(node->DataType) << " " << NextLocal(node) << " = " << valStr << ";" << std::endl;
}

void CHLSLCodeGen::Visit(IRUnaryOp* node)
{
	if (IsLocalAlready(node))
		return;

	node->Right->Accept(*this);

	CodeStream << "\t" << ConvertToHLSLType(node->DataType) << " " << NextLocal(node) << " = ";
	auto right = NodeToLocalVar(node->Right);

	switch (node->Op)
	{
	case IRUnaryOp::EOp::Neg:
		CodeStream << "-" + right;
		break;
	case IRUnaryOp::EOp::Sqrt:
		CodeStream << "sqrt(" + right + ")";
		break;
	case IRUnaryOp::EOp::Sin:
		CodeStream << "sin(" + right + ")";
		break;
	case IRUnaryOp::EOp::Cos:
		CodeStream << "cos(" + right + ")";
		break;
	default:
		throw CEffiCompileError("Error: Unary Op invalid");
		break;
	}
	CodeStream << ";" << std::endl;
}

void CHLSLCodeGen::Visit(IRAdd* node)
{
	if (IsLocalAlready(node))
		return;

	node->Left->Accept(*this);
	node->Right->Accept(*this);

	CodeStream << "\t" << ConvertToHLSLType(node->DataType) << " " << NextLocal(node) << " = " <<
		NodeToLocalVar(node->Left) << " + " << NodeToLocalVar(node->Right) <<
		";" << std::endl;
}

void CHLSLCodeGen::Visit(IRSub* node)
{
	if (IsLocalAlready(node))
		return;

	node->Left->Accept(*this);
	node->Right->Accept(*this);

	CodeStream << "\t" << ConvertToHLSLType(node->DataType) << " " << NextLocal(node) << " = " <<
		NodeToLocalVar(node->Left) << " - " << NodeToLocalVar(node->Right) <<
		";" << std::endl;
}

void CHLSLCodeGen::Visit(IRMul* node)
{
	if (IsLocalAlready(node))
		return;

	node->Left->Accept(*this);
	node->Right->Accept(*this);

	CodeStream << "\t" << ConvertToHLSLType(node->DataType) << " " << NextLocal(node) << " = " <<
		"mul(" << NodeToLocalVar(node->Left) << ", " << NodeToLocalVar(node->Right) << ")" <<
		";" << std::endl;
}

void CHLSLCodeGen::Visit(IRDiv* node)
{
	if (IsLocalAlready(node))
		return;

	node->Left->Accept(*this);
	node->Right->Accept(*this);

	CodeStream << "\t" << ConvertToHLSLType(node->DataType) << " " << NextLocal(node) << " = " <<
		NodeToLocalVar(node->Left) << " / " << NodeToLocalVar(node->Right) <<
		";" << std::endl;
}

void CHLSLCodeGen::Visit(IRDotProduct* node)
{
	if (IsLocalAlready(node))
		return;

	node->Left->Accept(*this);
	node->Right->Accept(*this);

	CodeStream << "\t" << ConvertToHLSLType(node->DataType) << " " << NextLocal(node) << " = " <<
		"dot(" << NodeToLocalVar(node->Left) << ", " << NodeToLocalVar(node->Right) << ")" <<
		";" << std::endl;
}

void CHLSLCodeGen::Visit(IRCrossProduct* node)
{
	if (IsLocalAlready(node))
		return;

	node->Left->Accept(*this);
	node->Right->Accept(*this);

	CodeStream << "\t" << ConvertToHLSLType(node->DataType) << " " << NextLocal(node) << " = " <<
		"cross(" << NodeToLocalVar(node->Left) << ", " << NodeToLocalVar(node->Right) << ")" <<
		";" << std::endl;
}

void CHLSLCodeGen::Visit(IRConstructMat3* node)
{
	if (IsLocalAlready(node))
		return;
	for (int i = 0; i < 9; i++)
		node->Args[i]->Accept(*this);
	CodeStream << "\t" << ConvertToHLSLType(node->DataType) << " " << NextLocal(node) << " = float3x3(";
	for (int i = 0; i < 9; i++)
	{
		CodeStream << NodeToLocalVar(node->Args[i]);
		if (i != 9 - 1)
			CodeStream << ", ";
	}
	CodeStream << ");" << std::endl;
}

std::string CHLSLCodeGen::NextLocal(IRNode* node)
{
	int num = NodeToLocal.size();
	NodeToLocal[node] = num;
	return "local" + std::to_string(num);
}

std::string CHLSLCodeGen::NodeToLocalVar(IRNode* node)
{
	return "local" + std::to_string(NodeToLocal[node]);
}

bool CHLSLCodeGen::IsLocalAlready(IRNode* node)
{
	auto iter = NodeToLocal.find(node);
	return iter != NodeToLocal.end();
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

CEffiCompiledPipeline* CEffiCompiler::Compile(IRProgram* program)
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
		return CompiledPipeline;
	}
	catch (const CEffiCompileError& e)
	{
		printf("ERROR during compilation: %s\n", e.what());
		return nullptr;
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

void CEffiCompiler::Visit(IRInputAttrFloat* node)
{
	RetExpr = node;
}

void CEffiCompiler::Visit(IRInputAttrVec3* node)
{
	RetExpr = node;
}

void CEffiCompiler::Visit(IRInputAttrVec4* node)
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

void CEffiCompiler::Visit(IRUnaryOp* node)
{
	GenericVisit(node->Right);
	node->Right = RetExpr;

	node->DataType = node->Right->DataType;
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

void CEffiCompiler::Visit(IRSub* node)
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
	throw CEffiCompileError("Invalid sub, type mismatch.");
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
	if (node->Left->DataType == EDataType::Float3 && node->Right->DataType == EDataType::Float)
	{
		node->DataType = node->Left->DataType;
		return;
	}
	if (node->Left->DataType == EDataType::Float4 && node->Right->DataType == EDataType::Float)
	{
		node->DataType = node->Left->DataType;
		return;
	}
	throw CEffiCompileError("Invalid mul, type mismatch.");
}

void CEffiCompiler::Visit(IRDiv* node)
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
	if (node->Left->DataType == EDataType::Float3 && node->Right->DataType == EDataType::Float)
	{
		node->DataType = node->Left->DataType;
		return;
	}
	if (node->Left->DataType == EDataType::Float4 && node->Right->DataType == EDataType::Float)
	{
		node->DataType = node->Left->DataType;
		return;
	}
	throw CEffiCompileError("Invalid div, type mismatch.");
}

void CEffiCompiler::Visit(IRDotProduct* node)
{
	//Recurse down
	GenericVisit(node->Left);
	node->Left = RetExpr;
	GenericVisit(node->Right);
	node->Right = RetExpr;

	//Type check, and return
	RetExpr = node;
	if (node->Left->DataType == EDataType::Float3 && node->Right->DataType == EDataType::Float3)
	{
		node->DataType = EDataType::Float; //dot product returns scalar
		return;
	}
	if (node->Left->DataType == EDataType::Float4 && node->Right->DataType == EDataType::Float4)
	{
		node->DataType = EDataType::Float; //dot product returns scalar
		return;
	}
	throw CEffiCompileError("Invalid dot, type mismatch.");
}

void CEffiCompiler::Visit(IRCrossProduct* node)
{
	//Recurse down
	GenericVisit(node->Left);
	node->Left = RetExpr;
	GenericVisit(node->Right);
	node->Right = RetExpr;

	//Type check, and return
	RetExpr = node;
	if (node->Left->DataType == EDataType::Float3 && node->Right->DataType == EDataType::Float3)
	{
		node->DataType = EDataType::Float3; //dot product returns scalar
		return;
	}
	if (node->Left->DataType == EDataType::Float4 && node->Right->DataType == EDataType::Float4)
	{
		node->DataType = EDataType::Float4; //dot product returns scalar
		return;
	}
	throw CEffiCompileError("Invalid cross, type mismatch.");
}

void CEffiCompiler::Visit(IRConstructMat3* node)
{
	//Recurse down
	for (int i = 0; i < 9; i++)
	{
		GenericVisit(node->Args[i]);
		node->Args[i] = RetExpr;
	}

	//Type check, and return
	RetExpr = node;
	node->DataType = EDataType::Float3x3;
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

	//node->Target is the reference to the attribute
	//iter->second is the IRExpr for the attribute
	MOMaterializeAttr* meshOp = new MOMaterializeAttr(EffiContext->GetGraphicsDevice(), node->Target->Name, iter->second);
	CompiledPipeline->AddOperator(meshOp);

	//Update the attribute table, must happen after building the Operator
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
