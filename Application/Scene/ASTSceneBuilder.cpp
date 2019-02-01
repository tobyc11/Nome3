#include "ASTSceneBuilder.h"
#include "Surface.h"
#include "Point.h"
#include "Polyline.h"
#include "Tunnel.h"
#include "Funnel.h"
#include "Circle.h"
#include <StringPrintf.h>

namespace Nome::Scene
{

#define CONNECT_AST_EXPR_TO(expr, to) \
do { \
	CExprToNodeGraph converter{ expr, Scene->GetBankAndSet() }; \
	converter.Connect(to); \
} while (0)

//Note:
//  Parser, AST, and the scene builder all have to match for any command
void CASTSceneBuilder::VisitCommand(ACommand* cmd)
{
	Visit(cmd);
}

void CASTSceneBuilder::VisitPoint(AIdent* name, AExpr* x, AExpr* y, AExpr* z)
{
	auto* point = new CPoint(name->Identifier);
	CONNECT_AST_EXPR_TO(x, point->X);
	CONNECT_AST_EXPR_TO(y, point->Y);
	CONNECT_AST_EXPR_TO(z, point->Z);
	Scene->AddEntity(point);
}

void CASTSceneBuilder::VisitPolyline(AIdent* name, const std::vector<AIdent*>& points, bool closed)
{
	TAutoPtr<CPolyline> polyline = new CPolyline(name->Identifier);
	for (auto* ident : points)
	{
		Flow::TOutput<CVertexInfo*>* pointOutput = Scene->FindPointOutput(ident->Identifier);
		if (!pointOutput)
		{
			throw std::runtime_error(tc::StringPrintf("Cannot find point %s", ident->Identifier.c_str()));
		}
		polyline->Points.Connect(*pointOutput);
	}
	polyline->SetClosed(closed);
	Scene->AddEntity(polyline.Get());
}

void CASTSceneBuilder::VisitFace(AIdent* name, const std::vector<AIdent*>& points, AIdent* surface)
{
	TAutoPtr<CFace> face = new CFace(EntityNamePrefix + name->Identifier);
	for (auto* ident : points)
	{
		Flow::TOutput<CVertexInfo*>* pointOutput = Scene->FindPointOutput(ident->Identifier);
		if (!pointOutput)
		{
			throw std::runtime_error(tc::StringPrintf("Cannot find point %s", ident->Identifier.c_str()));
		}
		face->Points.Connect(*pointOutput);
	}
	Scene->AddEntity(face.Get());

	if (InMesh)
	{
		InMesh->Faces.Connect(face->Face);
	}
	//TODO: handle surface
}

void CASTSceneBuilder::VisitObject(AIdent* name, const std::vector<AIdent*>& faceRefs)
{
	TAutoPtr<CMesh> mesh = new CMesh(name->Identifier);
	for (auto* ident : faceRefs)
	{
		TAutoPtr<CEntity> entity = Scene->FindEntity(ident->Identifier);
		if (!entity)
		{
			throw std::runtime_error(tc::StringPrintf("Cannot find entity %s", ident->Identifier.c_str()));
		}
		CFace* face = dynamic_cast<CFace*>(entity.Get());
		if (!face)
			throw std::runtime_error(tc::StringPrintf("Entity %s is not a face", ident->Identifier.c_str()));
		mesh->Faces.Connect(face->Face);
	}
	Scene->AddEntity(mesh.Get());
}

void CASTSceneBuilder::VisitMesh(AIdent* name, const std::vector<ACommand*>& faces)
{
	InMesh = new CMesh(name->Identifier);
	Scene->AddEntity(InMesh);
	EntityNamePrefix = name->Identifier + ".";
	for (ACommand* cmd : faces)
		Visit(cmd);
	EntityNamePrefix = "";
	InMesh = nullptr;
}

void CASTSceneBuilder::VisitGroup(AIdent* name, const std::vector<ACommand*>& instances)
{
	InstanciateUnder = Scene->CreateGroup(name->Identifier);
	for (ACommand* cmd : instances)
		Visit(cmd);
	InstanciateUnder = Scene->GetRootNode();
}

void CASTSceneBuilder::VisitCircle(AIdent* name, AExpr* n, AExpr* ro)
{
	auto* circle = new CCircle(name->Identifier);
	CONNECT_AST_EXPR_TO(n, circle->Segments);
	CONNECT_AST_EXPR_TO(ro, circle->Radius);
	Scene->AddEntity(circle);
}

void CASTSceneBuilder::VisitFunnel(AIdent* name, AExpr* n, AExpr* ro, AExpr* ratio, AExpr* h)
{
	auto* funnel = new CFunnel(name->Identifier);
	CONNECT_AST_EXPR_TO(n, funnel->VerticesPerRing);
	CONNECT_AST_EXPR_TO(ro, funnel->Radius);
	CONNECT_AST_EXPR_TO(ratio, funnel->Ratio);
	CONNECT_AST_EXPR_TO(h, funnel->Height);
	Scene->AddEntity(funnel);
}

void CASTSceneBuilder::VisitTunnel(AIdent* name, AExpr* n, AExpr* ro, AExpr* ratio, AExpr* h)
{
	auto* tunnel = new CTunnel(name->Identifier);
	CONNECT_AST_EXPR_TO(n, tunnel->VerticesPerRing);
	CONNECT_AST_EXPR_TO(ro, tunnel->Radius);
	CONNECT_AST_EXPR_TO(ratio, tunnel->Ratio);
	CONNECT_AST_EXPR_TO(h, tunnel->Height);
	Scene->AddEntity(tunnel);
}

void CASTSceneBuilder::VisitBezierCurve(AIdent* name, const std::vector<AIdent*>& points, AExpr* nSlices)
{
}

void CASTSceneBuilder::VisitBSpline(AIdent* name, const std::vector<AIdent*>& points, AExpr* order, AExpr* nSlices, bool closed)
{
	throw std::runtime_error("Sorry, bspline is not yet supported");
}

void CASTSceneBuilder::VisitInstance(AIdent* name, AIdent* entityName, const std::vector<ATransform*>& transformList, AIdent* surface)
{
	TAutoPtr<CTransform> lastTransform;
	for (ATransform* astTransform : transformList)
	{
		auto* transform = ConvertASTTransform(astTransform);
		if (lastTransform)
			transform->Input.Connect(lastTransform->Output);
		lastTransform = transform;
	}

	auto entity = Scene->FindEntity(entityName->Identifier);
	if (entity)
	{
		auto* sceneNode = InstanciateUnder->CreateChildNode(name->Identifier);
		if (lastTransform)
			sceneNode->Transform.Connect(lastTransform->Output);
		sceneNode->SetEntity(entity);
	}
	else if (auto group = Scene->FindGroup(entityName->Identifier))
	{
		auto* sceneNode = InstanciateUnder->CreateChildNode(name->Identifier);
		if (lastTransform)
			sceneNode->Transform.Connect(lastTransform->Output);
		group->AddParent(sceneNode);
	}
	else
	{
		throw std::runtime_error(tc::StringPrintf("Instantiation failed, unknown generator: %s", entityName->Identifier.c_str()));
	}
}

void CASTSceneBuilder::VisitSurface(AIdent* name, AExpr* r, AExpr* g, AExpr* b)
{
	auto* surface = new CSurface();
	surface->SetName(name->Identifier);
	CONNECT_AST_EXPR_TO(r, surface->ColorR);
	CONNECT_AST_EXPR_TO(g, surface->ColorG);
	CONNECT_AST_EXPR_TO(b, surface->ColorB);
	Scene->AddEntity(surface);
}

void CASTSceneBuilder::VisitBank(AIdent* name, const std::vector<ACommand*>& sets)
{
	for (auto* cmd : sets)
	{
		auto* value = ast_as<ANumber*>(cmd->Args[0]);
		auto* min = ast_as<ANumber*>(cmd->Args[1]);
		auto* max = ast_as<ANumber*>(cmd->Args[2]);
		auto* step = ast_as<ANumber*>(cmd->Args[3]);
		Scene->GetBankAndSet().AddSlider(name->Identifier + "." + cmd->Name->Identifier,
			CCommandHandle{ cmd, SourceManager, SourceFile },
			(float)value->GetValue(), (float)min->GetValue(),
			(float)max->GetValue(), (float)step->GetValue());
	}
}

void CASTSceneBuilder::VisitDelete(const std::vector<ACommand*>& faceCmds)
{
	for (auto* cmd : faceCmds)
	{
		if (cmd->BeginKeyword->Keyword != "face")
			throw std::runtime_error("Delete command can only contain faces.");
		auto node_face = Scene->WalkPath(cmd->Name->Identifier);
		CEntity* instEnt = node_face.first->GetInstanceEntity();
		if (!instEnt)
			throw std::runtime_error("Delete face names an invalid mesh instance");
		auto* meshInst = dynamic_cast<CMeshInstance*>(instEnt);
		if (!meshInst)
			throw std::runtime_error("Delete face names an invalid mesh instance");
		meshInst->GetFacesToDelete().insert(node_face.second);
		printf("[Debug] %s", node_face.second.c_str());
	}
}

TAutoPtr<CScene> CASTSceneBuilder::GetScene() const
{
	return Scene;
}

CTransform* CASTSceneBuilder::ConvertASTTransform(ATransform* t) const
{
	switch (t->Type)
	{
	case ATransform::TF_TRANSLATE:
	{
		auto* transform = new CTranslate();
		CONNECT_AST_EXPR_TO(t->AxisX, transform->X);
		CONNECT_AST_EXPR_TO(t->AxisY, transform->Y);
		CONNECT_AST_EXPR_TO(t->AxisZ, transform->Z);
		return transform;
	}
	case ATransform::TF_ROTATE:
	{
		auto* transform = new CRotate();
		CONNECT_AST_EXPR_TO(t->AxisX, transform->AxisX);
		CONNECT_AST_EXPR_TO(t->AxisY, transform->AxisY);
		CONNECT_AST_EXPR_TO(t->AxisZ, transform->AxisZ);
		CONNECT_AST_EXPR_TO(t->Deg, transform->Angle);
		return transform;
	}
	case ATransform::TF_SCALE:
	{
		auto* transform = new CScale();
		CONNECT_AST_EXPR_TO(t->AxisX, transform->X);
		CONNECT_AST_EXPR_TO(t->AxisY, transform->Y);
		CONNECT_AST_EXPR_TO(t->AxisZ, transform->Z);
		return transform;
	}
	}
	return nullptr;
}

CExprToNodeGraph::CExprToNodeGraph(AExpr* expr, CBankAndSet& bankAndSet) : BankAndSet(bankAndSet)
{
	Visit(expr);
}

void CExprToNodeGraph::VisitIdent(AIdent* ident)
{
	SliderVal = BankAndSet.GetSlider(ident->Identifier);
	if (!SliderVal)
		throw std::runtime_error("wtf no such slider");
	WhichOne = 7;
}

void CExprToNodeGraph::VisitNumber(ANumber* number)
{
	Number = new Flow::CFloatNumber((float)number->GetValue());
	WhichOne = 0;
}

void CExprToNodeGraph::VisitUnaryOp(AUnaryOp* op)
{
	switch (op->Type)
	{
	case AUnaryOp::UOP_NEG:
	{
		auto* curr = new Flow::CFloatNeg();
		Visit(op->Operand); //Visit the operand
		Connect(curr->Operand0); //Connect the operand to Negate's input
		Negate = curr;
		WhichOne = 1;
		break;
	}
	default:
		throw std::runtime_error("wtf unary operator unheard of");
		break;
	}
}

void CExprToNodeGraph::VisitBinaryOp(ABinaryOp* op)
{
	switch (op->Type)
	{
	case ABinaryOp::BOP_ADD:
	{
		auto* curr = new Flow::CFloatAdd();
		Visit(op->Left);
		Connect(curr->Operand0);
		Visit(op->Right);
		Connect(curr->Operand1);
		Add = curr;
		WhichOne = 2;
		break;
    }
	case ABinaryOp::BOP_SUB:
	{
		auto* curr = new Flow::CFloatSub();
		Visit(op->Left);
		Connect(curr->Operand0);
		Visit(op->Right);
		Connect(curr->Operand1);
		Sub = curr;
		WhichOne = 3;
		break;
    }
	case ABinaryOp::BOP_MUL:
	{
		auto* curr = new Flow::CFloatMul();
		Visit(op->Left);
		Connect(curr->Operand0);
		Visit(op->Right);
		Connect(curr->Operand1);
		Mul = curr;
		WhichOne = 4;
		break;
    }
	case ABinaryOp::BOP_DIV:
	{
		auto* curr = new Flow::CFloatDiv();
		Visit(op->Left);
		Connect(curr->Operand0);
		Visit(op->Right);
		Connect(curr->Operand1);
		Div = curr;
		WhichOne = 5;
		break;
    }
	case ABinaryOp::BOP_EXP:
	{
		auto* curr = new Flow::CFloatPow();
		Visit(op->Left);
		Connect(curr->Operand0);
		Visit(op->Right);
		Connect(curr->Operand1);
		Pow = curr;
		WhichOne = 6;
		break;
    }
	}
}

void CExprToNodeGraph::Connect(Flow::TInput<float>& input)
{
	switch (WhichOne)
	{
	case 0:
		Number->Value.Connect(input);
		break;
	case 1:
		Negate->Result.Connect(input);
		break;
	case 2:
		Add->Result.Connect(input);
		break;
	case 3:
		Sub->Result.Connect(input);
		break;
	case 4:
		Mul->Result.Connect(input);
		break;
	case 5:
		Div->Result.Connect(input);
		break;
	case 6:
		Pow->Result.Connect(input);
		break;
	case 7:
		SliderVal->Value.Connect(input);
		break;
	default:
		break;
	}
}

}
