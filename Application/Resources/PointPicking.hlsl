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
    uint Id : ATTRIBUTE2;
};

struct VSOut
{
    float4 Pos : SV_Position;
    uint Id : COLOR;
};

struct GSOut
{
    float4 Pos : SV_Position;
    float2 UV : TEXCOORD0;
    uint Id : COLOR;
};

VSOut VSmain(VSIn input)
{
    VSOut output;
    float4 worldPos = mul(float4(input.Pos, 1), Model);
    output.Pos = mul(mul(worldPos, View), Proj);
    output.Id = input.Id;
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

    output.Id = input[0].Id;
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

uint PSmain(GSOut input) : SV_Target0
{
    //Only the alpha channel is significant
    float4 rgba = PointTexture.Sample(PointSampler, input.UV);

    if (rgba.a > 0.5)
    {
        return input.Id;
    }
    discard;
    return 0;
}
