
#include "Face.h"
#include "Mesh.h"


namespace Nome::Scene
{

DEFINE_META_OBJECT(CSharp)
{
    BindPositionalArgument(&CSharp::Sharpness, 1);
    BindPositionalArgument(&CSharp::Points, 0);
}

void CSharp::MarkDirty()
{
    // Mark this entity dirty
    Super::MarkDirty();

    // And also mark the Sharp points output dirty
    SharpPoints.MarkDirty();
}

void CSharp::UpdateEntity()
{
    Super::UpdateEntity();
    SharpPoints.UpdateValue(this);
    SetValid(true);
}

bool CSharp::AddSharpnessIntoMesh(CMesh* mesh) const
{
    std::vector<std::string> nameList;
    for (size_t i = 0; i < Points.GetSize(); i++)
    {
        auto* point = Points.GetValue(i, nullptr);

        // point update failed etc
        if (!point)
        {
            perror ("can't add find the point the add sharpness!\n");
            return false;
        }

        if (mesh->HasVertex(point->Name))
        {
            nameList.push_back(point->Name);
        }

    }

    if (nameList.size() == 1) {
        mesh->AddPointSharpness(mesh->FindVertex(nameList.at(0)), Sharpness.GetValue(0.0f));
    } else if (nameList.size() > 1){
        for (int i = 0; i < nameList.size() - 1; ++i)
        {
            mesh->AddEdgeSharpness(mesh->FindVertex(nameList.at(i)), mesh->FindVertex(nameList.at(i + 1)), Sharpness.GetValue(0.0f));
        }
    }
    return true;
}



}