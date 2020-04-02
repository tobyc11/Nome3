#include "ControlPoint.h"

namespace Nome::Scene
{

DEFINE_META_OBJECT(CControlPoint)
{
    BindPositionalArgument(&CControlPoint::OwnerName, 1, 0);
}

}
