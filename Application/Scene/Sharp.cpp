
#include "Face.h"
#include "Mesh.h"


namespace Nome::Scene
{

DEFINE_META_OBJECT(CSharp)
{
    BindPositionalArgument(&CSharp::Sharpness, 1);
    BindPositionalArgument(&CSharp::SharpPoints, 2);
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



}