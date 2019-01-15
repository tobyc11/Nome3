#pragma once
#include "Scene.h"
#include <Parsing/SourceManager.h>

namespace Nome::Scene
{

class CSceneModifier
{
public:
	CSceneModifier(CScene* scene, CSourceFile* file, CASTContext* syntaxTree)
		: Scene(scene), File(file), AST(syntaxTree)
	{
	}

	void AddPoint(const std::string& name, const std::string& x, const std::string& y, const std::string& z);

	void AddInstance(const std::string& name, const std::string& entityName);

private:
	CScene* Scene;
	CSourceFile* File;
	CASTContext* AST;
};

}
