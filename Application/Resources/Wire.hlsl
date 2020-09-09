struct VSIn
{
    float3 Pos : ATTRIBUTE0;
};

struct VSOut
{
    float4 Pos : SV_Position;
};

cbuffer CBEverything : register(b0)
{
    float4x4 Model;
    float4x4 View;
    float4x4 Proj;
    float4 Color;
    float LineWidth;
    float Width;
    float Height;
    float Padding;
}

VSOut VSmain(VSIn input)
{
    VSOut output;
    output.Pos = mul(mul(mul(float4(input.Pos, 1), Model), View), Proj);
    return output;
}

void WidenLineSegment(VSOut p, VSOut q, inout TriangleStream<VSOut> OutputStream)
{
    VSOut output = (VSOut)0;
    //Do perspective division to get 2D coordinates
    float2 ndfP = float2(p.Pos.x / p.Pos.w, p.Pos.y / p.Pos.w);
    float2 ndfQ = float2(q.Pos.x / q.Pos.w, q.Pos.y / q.Pos.w);
    float2 pq = normalize(ndfQ - ndfP);
    float2 perp = float2(-pq.y * LineWidth / Width, pq.x * LineWidth / Height); //Vector perp to the line
    //float2 perp = float2(-pq.y, pq.x); //Vector perp to the line
    float4 deltaP = float4(perp, 0, 0) * p.Pos.w;
    float4 deltaQ = float4(perp, 0, 0) * q.Pos.w;
    output.Pos = p.Pos - deltaP;
    OutputStream.Append(output);
    output.Pos = p.Pos + deltaP;
    OutputStream.Append(output);
    output.Pos = q.Pos - deltaQ;
    OutputStream.Append(output);
    output.Pos = q.Pos + deltaQ;
    OutputStream.Append(output);
    OutputStream.RestartStrip();
}

[maxvertexcount(12)]
void GSmain(triangle VSOut input[3], inout TriangleStream<VSOut> OutputStream)
{
    WidenLineSegment(input[0], input[1], OutputStream);
    WidenLineSegment(input[1], input[2], OutputStream);
    WidenLineSegment(input[2], input[0], OutputStream);
}

float4 PSmain(VSOut input) : SV_Target0
{
    return Color;
}
