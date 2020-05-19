#pragma once
#include "Scene.h"
#include <Parsing/ASTContext.h>
#include <Parsing/SourceManager.h>
#include <string>
#include <vector>

namespace Nome::Scene
{

class CMesh;

class CTemporaryMeshManager
{
public:
    CTemporaryMeshManager(const TAutoPtr<CScene>& scene,
                          const std::shared_ptr<CSourceManager>& sourceMgr)
        : Scene(scene)
        , SourceMgr(sourceMgr)
    {
    }

    ~CTemporaryMeshManager() = default;

    void ResetTemporaryMesh();
    void AddFace(const std::vector<std::string>& facePoints);
    std::string CommitTemporaryMesh(AST::CASTContext& ctx, const std::string& entityName,
                                    const std::string& nodeName);

private:
    TAutoPtr<CScene> Scene;
    std::shared_ptr<CSourceManager> SourceMgr;

    CMesh* TempMesh = nullptr;
    CSceneNode* TempMeshNode = nullptr;
    unsigned int FaceCounter = 0;
};

}
