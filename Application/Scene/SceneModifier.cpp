#include "SceneModifier.h"
#include "ASTSceneBuilder.h"
#include "Point.h"
#include <Parsing/NomeDriver.h>
#include <sstream>

namespace Nome::Scene
{

void CSceneModifier::AddPoint(const std::string& name, const std::string& x, const std::string& y, const std::string& z)
{
    auto* point = new CPoint(name);

    //TODO: AST nodes produced this way do not have associated source locations
    auto parseAndConn = [this](const std::string& expr, Flow::TInput<float>& input)
    {
        CNomeDriver xD{ expr };
        xD.ParseToAST();
        CASTContext* xDAST = xD.GetASTContext();
        AExpr* xExpr = xDAST->GetExpr();
        CExprToNodeGraph xConverter{ xExpr, Scene->GetBankAndSet() };
        xConverter.Connect(input);
    };

    parseAndConn(x, point->X);
    parseAndConn(y, point->Y);
    parseAndConn(z, point->Z);

    Scene->AddEntity(point);

    std::stringstream ss;
    ss << "point " << name << " " << x << " " << y << " " << z << " endpoint";
    SourceManager->WriteLine(File, ss.str());
}

void CSceneModifier::AddInstance(const std::string& name, const std::string& entityName)
{
    auto ent = Scene->FindEntity(entityName);
    auto* node = Scene->GetRootNode()->CreateChildNode(name);
    node->SetEntity(ent);

    std::stringstream ss;
    ss << "instance " << name << " " << entityName << " endinstance";
    SourceManager->WriteLine(File, ss.str());
}

}
