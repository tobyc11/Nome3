#pragma once
#include "Scene.h"
#include <Parsing/ASTContext.h>
#include <Parsing/SourceManager.h>
#include <string>
#include <vector>
#include <map> // Steven's Add Point

namespace Nome::Scene
{

class CMesh;
class CPolyline;

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

    // void ResetTemporaryMesh(); Randy note: after 10/1, not used anymore as it's equivalent to
    // reloading a file

    void RemoveFace(const std::vector<std::string>&
                        faceNames); // Randy added this. Not sure if TemporaryMeshManager is best
                                    // place for it, but putting it here for now.
    void AddFace(const std::vector<std::string>& facePoints);

    void SelectOrDeselectPolyline(
        const std::vector<std::string>& points); // Randy added this for selection edges
    std::vector<std::string> AddedTempPolylineNodeNames; // added to assist with deselection

    void AddPolyline(const std::vector<std::string>& points);
    std::string AddPoint(const std::vector<std::string> pos); // Steven's function. Randy made it return the point name

    std::string CommitChanges(
        AST::CASTContext& ctx); // const std::string& entityName, const std::string& nodeName);
    std::vector<CSceneNode*> addedSceneNodes;
    std::vector<CMesh*> addedMeshes;
    std::vector<std::string>
        removedfaceNames; // randy added this on 10/30 to add removed face names to end of file

private:
    TAutoPtr<CScene> Scene;
    std::shared_ptr<CSourceManager> SourceMgr;

    CMesh* TempMesh = nullptr;
    CPolyline* TempPolyline = nullptr;

    CSceneNode* TempMeshNode = nullptr;
    CSceneNode* TempPolylineNode = nullptr;

    unsigned int FaceCounter = 0;
    unsigned int num_polylines = 0;

     // Steven add point
    CPolyline* TempPolylinePoint = nullptr;
    CSceneNode* TempPointNode = nullptr;
    CSceneNode* TempPolylinePointNode = nullptr;
    unsigned int num_points = 0;
    unsigned int polyline_prev_num_points = 0;
    std::map<std::string, std::string> pointMap;

};

}