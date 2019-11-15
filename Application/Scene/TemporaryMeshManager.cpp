#include "TemporaryMeshManager.h"

#include "Mesh.h"
#include <utility>

namespace Nome::Scene
{

CTemporaryMeshManager::CTemporaryMeshManager(TAutoPtr<CScene> scene)
    : Scene(std::move(scene))
{
}

void CTemporaryMeshManager::ResetTemporaryMesh()
{
    if (!TempMeshNode)
        TempMeshNode = Scene->GetRootNode()->CreateChildNode("__tempMeshNode");
    else
        TempMeshNode->SetEntity(nullptr);

    // Make entity and its corresponding scene node
    Scene->RemoveEntity("__tempMesh", true);
    TempMesh = new CMesh("__tempMesh");
    Scene->AddEntity(TempMesh);
    FaceCounter = 0;

    TempMeshNode->SetEntity(TempMesh);
}

void CTemporaryMeshManager::AddFace(const std::vector<std::string>& facePoints)
{
    if (!TempMesh && !TempMeshNode)
        ResetTemporaryMesh();

    // This function should roughly mirror the structure of CASTSceneBuilder::VisitFace
    const std::string entityNamePrefix = "__tempMesh.";
    const std::string faceName = "f" + std::to_string(FaceCounter);

    TAutoPtr<CFace> face = new CFace(entityNamePrefix + faceName);
    for (const auto& point : facePoints)
    {
        Flow::TOutput<CVertexInfo*>* pointOutput = Scene->FindPointOutput(point);
        if (!pointOutput)
        {
            // Uhh what happened?
            continue;
        }
        face->Points.Connect(*pointOutput);
    }
    Scene->AddEntity(tc::static_pointer_cast<CEntity>(face));

    TempMesh->Faces.Connect(face->Face);
}

void CTemporaryMeshManager::CommitTemporaryMesh(const std::string& entityName,
                                                const std::string& nodeName)
{
}

}
