#include "ASTSceneBuilder.h"
#include "Surface.h"
#include "Point.h"
#include "Polyline.h"

namespace Nome::Scene
{

//Note:
//  Parser, AST, and the scene builder all have to match for any command
void CASTSceneBuilder::VisitCommand(ACommand* cmd)
{
	Visit(cmd);
}

void CASTSceneBuilder::VisitPoint(AIdent* name, AExpr* x, AExpr* y, AExpr* z)
{
	auto* point = new CPoint(name->Identifier);
	CExprToNodeGraph arg0Conv{ x, Scene->GetBankAndSet() };
	arg0Conv.Connect(point->X);
	CExprToNodeGraph arg1Conv{ y, Scene->GetBankAndSet() };
	arg1Conv.Connect(point->Y);
	CExprToNodeGraph arg2Conv{ z, Scene->GetBankAndSet() };
	arg2Conv.Connect(point->Z);
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
			throw std::runtime_error("Cannot find point");
		}
		polyline->Points.Connect(*pointOutput);
	}
	Scene->AddEntity(polyline.Get());
	//TODO: handle closed
}

void CASTSceneBuilder::VisitFace(AIdent* name, const std::vector<AIdent*>& points, AIdent* surface)
{
	TAutoPtr<CFace> face = new CFace(EntityNamePrefix + name->Identifier);
	for (auto* ident : points)
	{
		Flow::TOutput<CVertexInfo*>* pointOutput = Scene->FindPointOutput(ident->Identifier);
		if (!pointOutput)
		{
			throw std::runtime_error("Cannot find point");
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
			throw std::runtime_error("Cannot find entity");
		}
		CFace* face = dynamic_cast<CFace*>(entity.Get());
		if (!face)
			throw std::runtime_error("Entity is not a face");
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
}

void CASTSceneBuilder::VisitFunnel(AIdent* name, AExpr* n, AExpr* ro, AExpr* ratio, AExpr* h)
{
}

void CASTSceneBuilder::VisitTunnel(AIdent* name, AExpr* n, AExpr* ro, AExpr* ratio, AExpr* h)
{
}

void CASTSceneBuilder::VisitBezierCurve(AIdent* name, const std::vector<AIdent*>& points, AExpr* nSlices)
{
}

void CASTSceneBuilder::VisitBSpline(AIdent* name, const std::vector<AIdent*>& points, AExpr* order, AExpr* nSlices, bool closed)
{
}

void CASTSceneBuilder::VisitInstance(AIdent* name, AIdent* entityName, const std::vector<ATransform*>& transformList, AIdent* surface)
{
	auto entity = Scene->FindEntity(entityName->Identifier);
	if (entity)
	{
		auto* sceneNode = InstanciateUnder->CreateChildNode(name->Identifier);
		sceneNode->SetEntity(entity);
	}
	else if (auto group = Scene->FindGroup(entityName->Identifier))
	{
		auto* sceneNode = InstanciateUnder->CreateChildNode(name->Identifier);
		group->AddParent(sceneNode);
	}
	else
	{
		throw std::runtime_error("Instantiation failed, unknown generator");
	}
}

void CASTSceneBuilder::VisitSurface(AIdent* name, AExpr* r, AExpr* g, AExpr* b)
{
	auto* surface = new CSurface();
	surface->SetName(name->Identifier);
	CExprToNodeGraph arg0Conv{ r, Scene->GetBankAndSet() };
	arg0Conv.Connect(surface->ColorR);
	CExprToNodeGraph arg1Conv{ g, Scene->GetBankAndSet() };
	arg1Conv.Connect(surface->ColorG);
	CExprToNodeGraph arg2Conv{ b, Scene->GetBankAndSet() };
	arg2Conv.Connect(surface->ColorB);
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
		Scene->GetBankAndSet().AddSlider(name->Identifier + "." + cmd->Name->Identifier, cmd,
			(float)value->GetValue(), (float)min->GetValue(),
			(float)max->GetValue(), (float)step->GetValue());
	}
}

void CASTSceneBuilder::VisitDelete(const std::vector<ACommand*>& faceCmds)
{
}

TAutoPtr<CScene> CASTSceneBuilder::GetScene() const
{
	return Scene;
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
