//Those are the contracts between the pipeline and the C++ program

cbuffer CBPerView : register(b0)
{
    float4x4 View;
    float4x4 Proj;
    float Width;
    float Height;
}

cbuffer CBPerObject : register(b1)
{
    float4x4 Model;
}

cbuffer CBPoint : register(b2)
{
    float PointRadius;
    bool bColorCodeByWorldPos;
}

Texture2D PointTexture : register(t0);
SamplerState PointSampler : register(s0);

struct VSIn
{
    float3 Pos : ATTRIBUTE0;
    float3 Color : ATTRIBUTE1;
};

struct VSOut
{
    float4 Pos : SV_Position;
    float4 Color : COLOR;
};

struct GSOut
{
    float4 Pos : SV_Position;
    float4 Color : COLOR;
    float2 UV : TEXCOORD0;
};

VSOut VSmain(VSIn input)
{
    VSOut output;
    float4 worldPos = mul(float4(input.Pos, 1), Model);
    output.Pos = mul(mul(worldPos, View), Proj);
    if (bColorCodeByWorldPos)
    {
        output.Color = (sin(worldPos) + 1) / 3;
    }
    else
    {
        output.Color = float4(input.Color, 0);
    }
    return output;
}

[maxvertexcount(4)]
void GSmain(point VSOut input[1], inout TriangleStream<GSOut> OutputStream)
{
    GSOut output = (GSOut)0;
    float4 p = input[0].Pos;
    float4 deltaX = float4(PointRadius * p.w / Width, 0, 0, 0);
    float4 deltaY = float4(0, PointRadius * p.w / Height, 0, 0);
    float4 p00 = p - deltaX - deltaY; float2 uv00 = float2(0, 0);
    float4 p01 = p - deltaX + deltaY; float2 uv01 = float2(0, 1);
    float4 p10 = p + deltaX - deltaY; float2 uv10 = float2(1, 0);
    float4 p11 = p + deltaX + deltaY; float2 uv11 = float2(1, 1);

    output.Color = input[0].Color;

    output.Pos = p00;
    output.UV = uv00;
    OutputStream.Append(output);
    output.Pos = p01;
    output.UV = uv01;
    OutputStream.Append(output);
    output.Pos = p10;
    output.UV = uv10;
    OutputStream.Append(output);
    output.Pos = p11;
    output.UV = uv11;
    OutputStream.Append(output);
    OutputStream.RestartStrip();
}

float4 PSmain(GSOut input) : SV_Target0
{
    //Only the alpha channel is significant
    float4 rgba = PointTexture.Sample(PointSampler, input.UV);

    float4 output = input.Color;
    output.a = rgba.a;
    return output;
}
