#include "Point.h"

namespace Nome::Scene
{

void CPoint::SetDefaultPosition(float x, float y, float z)
{
    DefaultX = x;
    DefaultY = y;
    DefaultZ = z;
    MarkDirty();
}

}
