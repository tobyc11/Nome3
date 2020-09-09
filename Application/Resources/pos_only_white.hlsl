#include <Generated.h>

struct VSIn
{
    float3 Pos : ATTRIBUTE0;
};

struct VSOut
{
    float4 Pos : SV_Position;
};

//cbuffer CBEverything
//{
//    float4x4 Model;
//    float4x4 View;
//    float4x4 Proj;
//};

VSOut VSmain(VSIn input)
{
    VSOut output;
    output.Pos = mul(Proj, mul(View, mul(Model, float4(input.Pos, 1))));
    return output;
}

float4 PSmain(VSOut input) : SV_Target0
{
    return float4(1, 1, 1, 1);
}