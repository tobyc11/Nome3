#pragma once
#include "Scene.h"
#include <Parsing/ASTContext.h>
#include <string>
#include <vector>

namespace Nome::Scene
{

class CMesh;

class CTemporaryMeshManager
{
public:
    explicit CTemporaryMeshManager(TAutoPtr<CScene> scene);
    ~CTemporaryMeshManager() = default;

    void ResetTemporaryMesh();
    void AddFace(const std::vector<std::string>& facePoints);
    std::string CommitTemporaryMesh(AST::CASTContext& ctx, const std::string& entityName, const std::string& nodeName);

private:
    TAutoPtr<CScene> Scene;

    CMesh* TempMesh = nullptr;
    CSceneNode* TempMeshNode = nullptr;
    unsigned int FaceCounter = 0;
};

}
