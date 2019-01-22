#pragma once
#include "AST.h"

namespace Nome
{

template <typename T>
class TASTConsumer
{
public:
    TASTConsumer(CASTContext& ast) : AST(ast)
    {
    }

    void Traverse()
    {
        const auto& cmds = AST.GetCommands();
		for (ACommand* cmd : cmds)
		{
			static_cast<T*>(this)->VisitCommand(cmd);
		}
    }

protected:
    CASTContext& AST;
};

template <typename T>
class TCommandVisitor
{
public:
	void Visit(ACommand* cmd)
	{
		if (cmd->BeginKeyword->Keyword == "point")
		{
			static_cast<T*>(this)->VisitPoint(cmd->Name, cmd->Args[0], cmd->Args[1], cmd->Args[2]);
		}
		else if (cmd->BeginKeyword->Keyword == "polyline")
		{
			//We assume the AST itself is valid
			std::vector<AIdent*> points;
			for (auto* expr : cmd->Args)
				points.push_back(static_cast<AIdent*>(expr));
			bool closed = cmd->FindNamedArg("closed");
			static_cast<T*>(this)->VisitPolyline(cmd->Name, points, closed);
		}
		else if (cmd->BeginKeyword->Keyword == "face")
		{
			std::vector<AIdent*> points;
			for (auto* expr : cmd->Args)
				points.push_back(static_cast<AIdent*>(expr));
			AIdent* surface = static_cast<AIdent*>(cmd->FindNamedArg("surface"));
			static_cast<T*>(this)->VisitFace(cmd->Name, points, surface);
		}
		else if (cmd->BeginKeyword->Keyword == "object")
		{
			std::vector<AIdent*> points;
			for (auto* expr : cmd->Args)
				points.push_back(static_cast<AIdent*>(expr));
			static_cast<T*>(this)->VisitObject(cmd->Name, points);
		}
		else if (cmd->BeginKeyword->Keyword == "mesh")
		{
			static_cast<T*>(this)->VisitMesh(cmd->Name, cmd->SubCommands);
		}
		else if (cmd->BeginKeyword->Keyword == "group")
		{
			static_cast<T*>(this)->VisitGroup(cmd->Name, cmd->SubCommands);
		}
		else if (cmd->BeginKeyword->Keyword == "circle")
		{
			static_cast<T*>(this)->VisitCircle(cmd->Name, cmd->Args[0], cmd->Args[1]);
		}
		else if (cmd->BeginKeyword->Keyword == "funnel")
		{
			static_cast<T*>(this)->VisitFunnel(cmd->Name, cmd->Args[0], cmd->Args[1], cmd->Args[2], cmd->Args[3]);
		}
		else if (cmd->BeginKeyword->Keyword == "tunnel")
		{
			static_cast<T*>(this)->VisitTunnel(cmd->Name, cmd->Args[0], cmd->Args[1], cmd->Args[2], cmd->Args[3]);
		}
		else if (cmd->BeginKeyword->Keyword == "beziercurve")
		{
			std::vector<AIdent*> points;
			for (auto* expr : cmd->Args)
				points.push_back(static_cast<AIdent*>(expr));
			AExpr* slices = cmd->FindNamedArg("slices");
			static_cast<T*>(this)->VisitBezierCurve(cmd->Name, points, slices);
		}
		else if (cmd->BeginKeyword->Keyword == "bspline")
		{
			std::vector<AIdent*> points;
			for (auto* expr : cmd->Args)
				points.push_back(static_cast<AIdent*>(expr));
			AExpr* order = cmd->FindNamedArg("order");
			AExpr* slices = cmd->FindNamedArg("slices");
			bool closed = cmd->FindNamedArg("closed");
			static_cast<T*>(this)->VisitBSpline(cmd->Name, points, order, slices, closed);
		}
		else if (cmd->BeginKeyword->Keyword == "instance")
		{
			//Args[0] is the entity; Args[1...] are the transforms
			std::vector<ATransform*> transforms;
			auto iter = cmd->Args.begin(), iterEnd = cmd->Args.end();
			for (++iter; iter != iterEnd; ++iter)
				transforms.push_back(static_cast<ATransform*>(*iter));

			AIdent* surface = static_cast<AIdent*>(cmd->FindNamedArg("surface"));

			static_cast<T*>(this)->VisitInstance(cmd->Name, static_cast<AIdent*>(cmd->Args[0]), transforms, surface);
		}
		else if (cmd->BeginKeyword->Keyword == "surface")
		{
			static_cast<T*>(this)->VisitSurface(cmd->Name, cmd->Args[0], cmd->Args[1], cmd->Args[2]);
		}
		else if (cmd->BeginKeyword->Keyword == "bank")
		{
			static_cast<T*>(this)->VisitBank(cmd->Name, cmd->SubCommands);
		}
		else if (cmd->BeginKeyword->Keyword == "delete")
		{
			static_cast<T*>(this)->VisitDelete(cmd->SubCommands);
		}
	}

	void VisitPoint(AIdent* name, AExpr* x, AExpr* y, AExpr* z) {}
	void VisitPolyline(AIdent* name, const std::vector<AIdent*>& points, bool closed) {}
	void VisitFace(AIdent* name, const std::vector<AIdent*>& points, AIdent* surface) {}
	void VisitObject(AIdent* name, const std::vector<AIdent*>& faceRefs) {}
	void VisitMesh(AIdent* name, const std::vector<ACommand*>& faces) {}
	void VisitGroup(AIdent* name, const std::vector<ACommand*>& instances) {}
	void VisitCircle(AIdent* name, AExpr* n, AExpr* ro) {}
	void VisitFunnel(AIdent* name, AExpr* n, AExpr* ro, AExpr* ratio, AExpr* h) {}
	void VisitTunnel(AIdent* name, AExpr* n, AExpr* ro, AExpr* ratio, AExpr* h) {}
	void VisitBezierCurve(AIdent* name, const std::vector<AIdent*>& points, AExpr* nSlices) {}
	void VisitBSpline(AIdent* name, const std::vector<AIdent*>& points, AExpr* order, AExpr* nSlices, bool closed) {}
	void VisitInstance(AIdent* name, AIdent* entityName, const std::vector<ATransform*>& transformList, AIdent* surface) {}
	void VisitSurface(AIdent* name, AExpr* r, AExpr* g, AExpr* b) {}
	void VisitBank(AIdent* name, const std::vector<ACommand*>& sets) {}
	void VisitDelete(const std::vector<ACommand*>& faceCmds) {}
};

template <typename T>
class TExprVisitor
{
public:
	void Visit(AExpr* expr)
	{
		if (auto* node = ast_as<AIdent*>(expr))
			static_cast<T*>(this)->VisitIdent(node);
		else if (auto* node = ast_as<ANumber*>(expr))
			static_cast<T*>(this)->VisitNumber(node);
		else if (auto* node = ast_as<AUnaryOp*>(expr))
			static_cast<T*>(this)->VisitUnaryOp(node);
		else if (auto* node = ast_as<ABinaryOp*>(expr))
			static_cast<T*>(this)->VisitBinaryOp(node);
	}

	void VisitIdent(AIdent* ident) {}
	void VisitNumber(ANumber* number) {}
	void VisitUnaryOp(AUnaryOp* op) {}
	void VisitBinaryOp(ABinaryOp* op) {}
};

}
