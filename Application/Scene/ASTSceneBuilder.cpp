#include "ASTSceneBuilder.h"
#include "Surface.h"
#include "Point.h"
#include "Polyline.h"

namespace Nome::Scene
{

//Note:
//  Parser, AST, and the scene builder all have to match for any command
void CASTSceneBuilder::BeginCommand(ACommand* cmd)
{
	printf("%s\n", cmd->BeginKeyword->Keyword.c_str());
	if (cmd->BeginKeyword->Keyword == "bank")
	{
		BankName = cmd->Name->Identifier;
	}
	else if (cmd->BeginKeyword->Keyword == "set")
	{
		auto* value = ast_as<ANumber*>(cmd->Args[0]);
		auto* min = ast_as<ANumber*>(cmd->Args[1]);
		auto* max = ast_as<ANumber*>(cmd->Args[2]);
		auto* step = ast_as<ANumber*>(cmd->Args[3]);
        Scene->GetBankAndSet().AddSlider(BankName + "." + cmd->Name->Identifier, cmd,
                                         (float)value->GetValue(), (float)min->GetValue(),
                                         (float)max->GetValue(), (float)step->GetValue());
	}
	else if (cmd->BeginKeyword->Keyword == "surface")
	{
		auto* surface = new CSurface();
		surface->SetName(cmd->Name->Identifier);
        CExprToNodeGraph arg0Conv{ cmd->Args[0], Scene->GetBankAndSet() };
		arg0Conv.Connect(surface->ColorR);
        CExprToNodeGraph arg1Conv{ cmd->Args[1], Scene->GetBankAndSet() };
		arg1Conv.Connect(surface->ColorG);
        CExprToNodeGraph arg2Conv{ cmd->Args[2], Scene->GetBankAndSet() };
		arg2Conv.Connect(surface->ColorB);
		Scene->AddEntity(surface);
	}
	else if (cmd->BeginKeyword->Keyword == "point")
	{
		auto* point = new CPoint(cmd->Name->Identifier);
        CExprToNodeGraph arg0Conv{ cmd->Args[0], Scene->GetBankAndSet() };
		arg0Conv.Connect(point->X);
        CExprToNodeGraph arg1Conv{ cmd->Args[1], Scene->GetBankAndSet() };
		arg1Conv.Connect(point->Y);
        CExprToNodeGraph arg2Conv{ cmd->Args[2], Scene->GetBankAndSet() };
		arg2Conv.Connect(point->Z);
		Scene->AddEntity(point);
	}
	else if (cmd->BeginKeyword->Keyword == "polyline")
	{
        TAutoPtr<CPolyline> polyline = new CPolyline(cmd->Name->Identifier);
        for (auto* expr : cmd->Args)
        {
            auto* ident = ast_as<AIdent*>(expr);
            Flow::TOutput<CVertexInfo*>* pointOutput = Scene->FindPointOutput(ident->Identifier);
            if (!pointOutput)
            {
                throw std::runtime_error("Cannot find point");
            }
            polyline->Points.Connect(*pointOutput);
        }
        Scene->AddEntity(polyline.Get());
	}
	else if (cmd->BeginKeyword->Keyword == "instance")
	{
        //Args[0] is the generator(entity) name, could also be a group
        auto* entityId = ast_as<AIdent*>(cmd->Args[0]);
        auto entity = Scene->FindEntity(entityId->Identifier);
        if (entity)
        {
            auto* sceneNode = Scene->GetRootNode()->CreateChildNode(cmd->Name->Identifier);
			sceneNode->SetEntity(entity);
        }
        else if (false /* is a group */)
        {
        }
        else
        {
            throw std::runtime_error("Instantiation failed, unknown generator");
        }
	}
	else if (cmd->BeginKeyword->Keyword == "group")
	{

	}
	else
	{
		printf("Encountered unknown command %s\n", cmd->BeginKeyword->Keyword.c_str());
	}
}

void CASTSceneBuilder::EndCommand(ACommand* cmd)
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
