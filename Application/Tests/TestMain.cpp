#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "Flow/Arithmetics.h"

TEST_CASE("Basic functionality of the dataflow graph")
{
    using namespace Flow;
    CFloatNumber input1{1.0f}; input1.AddRef();
    CFloatNumber input2{2.0f}; input2.AddRef();
    CFloatNumber input3{3.0f}; input3.AddRef();

    CFloatAdd adder; adder.AddRef();
    adder.Operand0.Connect(input1.Value);
    adder.Operand1.Connect(input2.Value);

    CFloatAdd adder2; adder2.AddRef();
    adder2.Operand0.Connect(input2.Value);
    adder2.Operand1.Connect(input3.Value);

    CFloatMul multiplier; multiplier.AddRef();
    multiplier.Operand0.Connect(adder.Result);
    multiplier.Operand1.Connect(adder2.Result);

    float fin = multiplier.Result.GetValue(-1.0f);
    REQUIRE(fin == 15.0f);

    input1.SetNumber(2.0f);
    fin = multiplier.Result.GetValue(-1.0f);
    REQUIRE(fin == 20.0f);
}
