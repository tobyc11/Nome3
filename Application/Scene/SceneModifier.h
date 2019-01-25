#pragma once
#include "Scene.h"
#include <Parsing/SourceManager.h>

namespace Nome::Scene
{

class CSceneModifier
{
public:
	CSceneModifier(CScene* scene, CSourceManager* sm, CSourceFile* file, CASTContext* syntaxTree)
		: Scene(scene), SourceManager(sm), File(file), AST(syntaxTree)
	{
	}

	void AddPoint(const std::string& name, const std::string& x, const std::string& y, const std::string& z);

	void AddInstance(const std::string& name, const std::string& entityName);

private:
	CScene* Scene;
	CSourceManager* SourceManager;
	CSourceFile* File;
	CASTContext* AST;
};

}
