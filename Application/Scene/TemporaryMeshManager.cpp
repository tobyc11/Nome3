#include "TemporaryMeshManager.h"

#include "Mesh.h"
#include <sstream>
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
    if (!TempMesh || !TempMeshNode)
        ResetTemporaryMesh();

    // This function should roughly mirror the structure of CASTSceneBuilder::VisitFace
    const std::string entityNamePrefix = "__tempMesh.";
    const std::string faceName = "f" + std::to_string(FaceCounter);

    TAutoPtr<CFace> face = new CFace(entityNamePrefix + faceName);
    face->SetPointSourceNames(Scene, facePoints);
    Scene->AddEntity(tc::static_pointer_cast<CEntity>(face));

    TempMesh->Faces.Connect(face->Face);
}

std::string CTemporaryMeshManager::CommitTemporaryMesh(const std::string& entityName,
                                                       const std::string& nodeName)
{
    if (!TempMesh || !TempMeshNode)
        return "";

    Scene->RenameEntity("__tempMesh", entityName);
    TempMeshNode->SetName(nodeName);

    std::stringstream ss;
    ss << "mesh " << entityName << std::endl;
    size_t count = TempMesh->Faces.GetSize();
    for (size_t i = 0; i < count; i++)
    {
        auto* face = TempMesh->Faces.GetValue(i, nullptr);
        ss << "    face " << face->GetNameWithoutPrefix() << " (";
        bool first = true;
        for (const auto& pointName : face->GetPointSourceNames())
        {
            if (first)
                first = false;
            else
                ss << " ";
            ss << pointName;
        }
        ss << ") endface" << std::endl;
    }
    ss << "endmesh" << std::endl;
    ss << "instance " << nodeName << " " << entityName << " endinstance" << std::endl;

    TempMesh = nullptr;
    TempMeshNode = nullptr;
    return ss.str();
}

}
