#pragma once
#include "AST.h"

namespace Nome
{

template <typename T> class TASTConsumer
{
public:
    TASTConsumer(CASTContext& ast)
        : AST(ast)
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

template <typename T> class TCommandVisitor
{
public:
    void Visit(ACommand* cmd)
    {
        if (cmd->GetBeginKeyword()->Identifier == "point")
        {
            auto* pos = ast_as<AExprList*>(cmd->FindNamedArg("position"));
            const auto& vec = pos->GetExpressions();
            static_cast<T*>(this)->VisitPoint(cmd->GetName(), vec[0], vec[1], vec[2]);
        }
        else if (cmd->GetBeginKeyword()->Identifier == "polyline")
        {
            // We assume the AST itself is valid
            auto* pointList = ast_as<AExprList*>(cmd->FindNamedArg("point_list"));
            auto idents = pointList->ConvertToIdents();
            bool closed = cmd->FindNamedArg("closed");
            static_cast<T*>(this)->VisitPolyline(cmd->GetName(), idents, closed);
        }
        else if (cmd->GetBeginKeyword()->Identifier == "sweep")
        {
            // We assume the AST itself is valid
            auto* pointList = ast_as<AExprList*>(cmd->FindNamedArg("point_list"));
            auto idents = pointList->ConvertToIdents();
            static_cast<T*>(this)->VisitSweep(cmd->GetName(), idents);
        }
        else if (cmd->GetBeginKeyword()->Identifier == "face")
        {
            auto* pointList = ast_as<AExprList*>(cmd->FindNamedArg("point_list"));
            auto idents = pointList->ConvertToIdents();
            AIdent* surface = static_cast<AIdent*>(cmd->FindNamedArg("surface"));
            static_cast<T*>(this)->VisitFace(cmd->GetName(), idents, surface);
        }
        else if (cmd->GetBeginKeyword()->Identifier == "object")
        {
            auto* exprList = ast_as<AExprList*>(cmd->FindNamedArg("primitives"));
            auto primitives = exprList->ConvertToIdents();
            static_cast<T*>(this)->VisitObject(cmd->GetName(), primitives);
        }
        else if (cmd->GetBeginKeyword()->Identifier == "mesh")
        {
            static_cast<T*>(this)->VisitMesh(cmd->GetName(), cmd->GatherSubcommands());
        }
        else if (cmd->GetBeginKeyword()->Identifier == "group")
        {
            static_cast<T*>(this)->VisitGroup(cmd->GetName(), cmd->GatherSubcommands());
        }
        else if (cmd->GetBeginKeyword()->Identifier == "circle")
        {
            static_cast<T*>(this)->VisitCircle(cmd->GetName(), cmd->FindNamedArg("subdiv"),
                                               cmd->FindNamedArg("radius"));
        }
        else if (cmd->GetBeginKeyword()->Identifier == "funnel")
        {
            static_cast<T*>(this)->VisitFunnel(
                cmd->GetName(), cmd->FindNamedArg("subdiv"), cmd->FindNamedArg("radius"),
                cmd->FindNamedArg("ratio"), cmd->FindNamedArg("height"));
        }
        else if (cmd->GetBeginKeyword()->Identifier == "tunnel")
        {
            static_cast<T*>(this)->VisitTunnel(
                cmd->GetName(), cmd->FindNamedArg("subdiv"), cmd->FindNamedArg("radius"),
                cmd->FindNamedArg("ratio"), cmd->FindNamedArg("height"));
        }
        else if (cmd->GetBeginKeyword()->Identifier == "beziercurve")
        {
            auto* exprList = ast_as<AExprList*>(cmd->FindNamedArg("point_list"));
            auto points = exprList->ConvertToIdents();
            AExpr* slices = cmd->FindNamedArg("slices");
            static_cast<T*>(this)->VisitBezierCurve(cmd->GetName(), points, slices);
        }
        else if (cmd->GetBeginKeyword()->Identifier == "bspline")
        {
            auto* exprList = ast_as<AExprList*>(cmd->FindNamedArg("point_list"));
            auto points = exprList->ConvertToIdents();
            AExpr* order = cmd->FindNamedArg("order");
            AExpr* slices = cmd->FindNamedArg("slices");
            bool closed = cmd->FindNamedArg("closed");
            static_cast<T*>(this)->VisitBSpline(cmd->GetName(), points, order, slices, closed);
        }
        else if (cmd->GetBeginKeyword()->Identifier == "instance")
        {
            auto* exprList = ast_as<AExprList*>(cmd->FindNamedArg("transformation"));
            std::vector<ATransform*> transforms;
            for (AExpr* expr : exprList->GetExpressions())
            {
                ATransform* ident = ast_as<ATransform*>(expr);
                if (!ident)
                    throw CSemanticError("Cannot convert to ATransform", expr);
                else
                    transforms.push_back(ident);
            }

            AIdent* surface = static_cast<AIdent*>(cmd->FindNamedArg("surface"));

            static_cast<T*>(this)->VisitInstance(cmd->GetName(),
                                                 static_cast<AIdent*>(cmd->FindNamedArg("target")),
                                                 transforms, surface);
        }
        else if (cmd->GetBeginKeyword()->Identifier == "surface")
        {
            auto* exprList = ast_as<AExprList*>(cmd->FindNamedArg("color"));
            static_cast<T*>(this)->VisitSurface(cmd->GetName(), exprList->GetExpressions()[0],
                                                exprList->GetExpressions()[1],
                                                exprList->GetExpressions()[2]);
        }
        else if (cmd->GetBeginKeyword()->Identifier == "bank")
        {
            static_cast<T*>(this)->VisitBank(cmd->GetName(), cmd->GatherSubcommands());
        }
        else if (cmd->GetBeginKeyword()->Identifier == "delete")
        {
            static_cast<T*>(this)->VisitDelete(cmd->GatherSubcommands());
        }
        else
        {
            printf("WARN: Unrecognized command %s at %d:%d\n",
                   cmd->GetBeginKeyword()->Identifier.c_str(),
                   cmd->GetBeginKeyword()->BeginLoc.DebugLine,
                   cmd->GetBeginKeyword()->BeginLoc.DebugCol);
        }
    }

    void VisitPoint(AIdent* name, AExpr* x, AExpr* y, AExpr* z) {}
    void VisitPolyline(AIdent* name, const std::vector<AIdent*>& points, bool closed) {}
    void VisitSweep(AIdent* name, const std::vector<AIdent*>& points) {}
    void VisitFace(AIdent* name, const std::vector<AIdent*>& points, AIdent* surface) {}
    void VisitObject(AIdent* name, const std::vector<AIdent*>& faceRefs) {}
    void VisitMesh(AIdent* name, const std::vector<ACommand*>& faces) {}
    void VisitGroup(AIdent* name, const std::vector<ACommand*>& instances) {}
    void VisitCircle(AIdent* name, AExpr* n, AExpr* ro) {}
    void VisitFunnel(AIdent* name, AExpr* n, AExpr* ro, AExpr* ratio, AExpr* h) {}
    void VisitTunnel(AIdent* name, AExpr* n, AExpr* ro, AExpr* ratio, AExpr* h) {}
    void VisitBezierCurve(AIdent* name, const std::vector<AIdent*>& points, AExpr* nSlices) {}
    void VisitBSpline(AIdent* name, const std::vector<AIdent*>& points, AExpr* order,
                      AExpr* nSlices, bool closed)
    {
    }
    void VisitInstance(AIdent* name, AIdent* entityName,
                       const std::vector<ATransform*>& transformList, AIdent* surface)
    {
    }
    void VisitSurface(AIdent* name, AExpr* r, AExpr* g, AExpr* b) {}
    void VisitBank(AIdent* name, const std::vector<ACommand*>& sets) {}
    void VisitDelete(const std::vector<ACommand*>& faceCmds) {}
};

template <typename T> class TExprVisitor
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
