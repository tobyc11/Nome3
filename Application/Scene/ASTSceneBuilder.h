#pragma once
#include "Scene.h"
#include <Parsing/ASTConsumer.h>
#include <Parsing/SourceManager.h>
#include <exception>
#include <string>

namespace Nome::Scene
{

// Forward declaration
class CMesh;

class CASTSceneBuilder : public TASTConsumer<CASTSceneBuilder>,
                         public TCommandVisitor<CASTSceneBuilder>
{
public:
    CASTSceneBuilder(CASTContext& ast, CSourceManager* sm, CSourceFile* sf)
        : TASTConsumer(ast)
        , SourceManager(sm)
        , SourceFile(sf)
    {
        Scene = new CScene();
        InstanciateUnder = Scene->GetRootNode();
    }

    void VisitCommand(ACommand* cmd);

    void VisitPoint(AIdent* name, AExpr* x, AExpr* y, AExpr* z);
    void VisitPolyline(AIdent* name, const std::vector<AIdent*>& points, bool closed);
    void VisitSweep(AIdent* name, const std::vector<AIdent*>& points);
    void VisitFace(AIdent* name, const std::vector<AIdent*>& points, AIdent* surface);
    void VisitObject(AIdent* name, const std::vector<AIdent*>& faceRefs);
    void VisitMesh(AIdent* name, const std::vector<ACommand*>& faces);
    void VisitGroup(AIdent* name, const std::vector<ACommand*>& instances);
    void VisitCircle(AIdent* name, AExpr* n, AExpr* ro);
    void VisitFunnel(AIdent* name, AExpr* n, AExpr* ro, AExpr* ratio, AExpr* h);
    void VisitTunnel(AIdent* name, AExpr* n, AExpr* ro, AExpr* ratio, AExpr* h);
    void VisitBezierCurve(AIdent* name, const std::vector<AIdent*>& points, AExpr* nSlices);
    void VisitBSpline(AIdent* name, const std::vector<AIdent*>& points, AExpr* order,
                      AExpr* nSlices, bool closed);
    void VisitInstance(AIdent* name, AIdent* entityName,
                       const std::vector<ATransform*>& transformList, AIdent* surface);
    void VisitSurface(AIdent* name, AExpr* r, AExpr* g, AExpr* b);
    void VisitBank(AIdent* name, const std::vector<ACommand*>& sets);
    void VisitDelete(const std::vector<ACommand*>& faceCmds);

    TAutoPtr<CScene> GetScene() const;

private:
    CTransform* ConvertASTTransform(ATransform* t) const;

    sp<CSourceManager> SourceManager;
    CSourceFile* SourceFile;

    TAutoPtr<CScene> Scene;

    // temporary variables used in traverse
    CSceneNode* InstanciateUnder;
    std::string EntityNamePrefix;
    CMesh* InMesh = nullptr;
};

// Converts an AST expression tree into a flow node graph
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

    // Boost::union and such is probably a better choice here for style, but curr impl is good
    // enough
    int WhichOne = 0;
    TAutoPtr<Flow::CFloatNumber> Number; // 0
    TAutoPtr<Flow::CFloatNeg> Negate; // 1
    TAutoPtr<Flow::CFloatSin> Sin; // 8
    TAutoPtr<Flow::CFloatCos> Cos; // 9
    TAutoPtr<Flow::CFloatAdd> Add; // 2
    TAutoPtr<Flow::CFloatSub> Sub; // 3
    TAutoPtr<Flow::CFloatMul> Mul; // 4
    TAutoPtr<Flow::CFloatDiv> Div; // 5
    TAutoPtr<Flow::CFloatPow> Pow; // 6
    TAutoPtr<Flow::CFloatNumber> SliderVal; // 7
};

}
