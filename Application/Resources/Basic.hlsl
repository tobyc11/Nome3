struct VSIn
{
    float3 Pos : ATTRIBUTE0;
    float3 Normal : ATTRIBUTE1;
};

struct VSOut
{
    float4 Pos : SV_Position;
    float3 Normal : NORMAL;
};

cbuffer CBEverything : register(b0)
{
    float4x4 Model;
    float4x4 View;
    float4x4 Proj;
    float4 Color;
}

VSOut VSmain(VSIn input)
{
    VSOut output;
    output.Pos = mul(mul(mul(float4(input.Pos, 1), Model), View), Proj);
    output.Normal = input.Normal;
    return output;
}

float3 L = float3(1, 1, 1);

float4 PSmain(VSOut input) : SV_Target0
{
    float cosWeight = dot(L, input.Normal);
    return float4(input.Normal * 0.5 + 0.5, 1.0);//Color * (0.5 + cosWeight);
}
