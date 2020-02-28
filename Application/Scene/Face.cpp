#include "Face.h"
#include "Mesh.h"

namespace Nome::Scene
{

DEFINE_META_OBJECT(CFace)
{
    BindPositionalArgument(&CFace::Points, 1);
    // Handle argSurface
    // Handle parent mesh connection
}

void CFace::MarkDirty()
{
    // Mark this entity dirty
    Super::MarkDirty();

    // And also mark the Face output dirty
    Face.MarkDirty();
}

void CFace::UpdateEntity()
{
    Super::UpdateEntity();
    Face.UpdateValue(this);
    SetValid(true);
}

size_t CFace::CountVertices() const { return Points.GetSize(); }

std::vector<std::string> CFace::GetPointSourceNames() const { return PointSource; }

void CFace::SetPointSourceNames(const TAutoPtr<CScene>& scene, std::vector<std::string> points)
{
    PointSource = std::move(points);

    for (const auto& point : PointSource)
    {
        Flow::TOutput<CVertexInfo*>* pointOutput = scene->FindPointOutput(point);
        if (!pointOutput)
        {
            // Uhh what happened?
            continue;
        }
        Points.Connect(*pointOutput);
    }
}

bool CFace::AddFaceIntoMesh(CMesh* mesh) const
{
    std::vector<std::string> nameList;
    for (size_t i = 0; i < Points.GetSize(); i++)
    {
        auto* point = Points.GetValue(i, nullptr);

        // point update failed etc
        if (!point)
        {
            return false;
        }

        std::string newName = point->Name;
        if (mesh->HasVertex(point->Name))
        {
            // See if those two points are close enough
            float dist = mesh->GetVertexPos(point->Name).DistanceToPoint(point->Position);
            const float epsilon = 0.001f;
            if (dist < epsilon)
            {
                // Skip adding this vertex
                nameList.push_back(point->Name);
                continue;
            }

            // Not close enough, rename and add the vertex
            int suffix = 1;
            while (mesh->HasVertex(newName))
                newName = point->Name + std::to_string(suffix++);
            printf("[Mesh: %s] Vertex %s has been renamed to %s\n", mesh->GetName().c_str(),
                   point->Name.c_str(), newName.c_str());
        }

        mesh->AddVertex(newName, point->Position);
        nameList.push_back(newName);
    }
    mesh->AddFace(GetName(), nameList);
    return true;
}

}
