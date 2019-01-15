#pragma once
#include "Scene.h"
#include <Parsing/ASTConsumer.h>
#include <string>

namespace Nome::Scene
{

class CASTSceneBuilder : public TASTConsumer<CASTSceneBuilder>
{
public:
	CASTSceneBuilder(CASTContext& ast) : TASTConsumer(ast)
	{
		Scene = new CScene();
	}

	void BeginCommand(ACommand* cmd);
	void EndCommand(ACommand* cmd);

	TAutoPtr<CScene> GetScene() const;

private:
	TAutoPtr<CScene> Scene;
	std::string BankName;
};

//Converts an AST expression tree into a flow node graph
class CExprToNodeGraph : public TExprVisitor<CExprToNodeGraph>
{
public:
    CExprToNodeGraph(AExpr* expr, CBankAndSet& bankAndSet);

	void VisitIdent(AIdent* ident);
	void VisitNumber(ANumber* number);
	void VisitUnaryOp(AUnaryOp* op);
	void VisitBinaryOp(ABinaryOp* op);

	void Connect(Flow::TInput<float>& input);

private:
	CBankAndSet& BankAndSet;

	//Boost::union and such is probably a better choice here for style, but curr impl is good enough
	int WhichOne = 0;
	TAutoPtr<Flow::CFloatNumber> Number; //0
	TAutoPtr<Flow::CFloatNeg> Negate; //1
	TAutoPtr<Flow::CFloatAdd> Add; //2
	TAutoPtr<Flow::CFloatSub> Sub; //3
	TAutoPtr<Flow::CFloatMul> Mul; //4
	TAutoPtr<Flow::CFloatDiv> Div; //5
	TAutoPtr<Flow::CFloatPow> Pow; //6
	TAutoPtr<Flow::CFloatNumber> SliderVal; //7
};

}
