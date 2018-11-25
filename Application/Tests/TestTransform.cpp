#include "Nome/Transforms.h"
#include "Flow/Arithmetics.h"

#include "catch.hpp"

TEST_CASE("Test CScale in Nome scene graph")
{
    using namespace Flow;
    using namespace Nome::Scene;
    using tc::TAutoPtr;

    TAutoPtr<CScale> scaleNode = new CScale();
    TAutoPtr<CFloatNumber> two = new CFloatNumber();
    two->SetNumber(2.0f);

    two->Value.Connect(scaleNode->Z);

    Matrix3x4 scaleMat = scaleNode->Output.GetValue(Matrix3x4::IDENTITY);

    Vector3 result = scaleMat * Vector3::ONE;

    REQUIRE(result == Vector3(1.0f, 1.0f, 2.0f));
}
