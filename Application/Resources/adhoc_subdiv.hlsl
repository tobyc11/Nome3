VSOut VSmain(VSIn input)
{
    VSOut output;
    output.Pos = CalcPos(input);
    return output;
}

[maxvertexcount(9)]
void GSmain(triangle VSOut input[3], inout TriangleStream<VSOut> TriStream)
{
    VSOut output;
    
    const float Explode = 0.128;

    //
    // Calculate the face normal
    //
    float3 faceEdgeA = input[1].Pos - input[0].Pos;
    float3 faceEdgeB = input[2].Pos - input[0].Pos;
    float3 faceNormal = normalize( cross(faceEdgeA, faceEdgeB) );
    float3 ExplodeAmt = faceNormal*Explode;
    
    //
    // Calculate the face center
    //
    float3 centerPos = (input[0].Pos.xyz + input[1].Pos.xyz + input[2].Pos.xyz)/3.0;
    centerPos += faceNormal*Explode;
    
    //
    // Output the pyramid
    //
    for(uint i=0; i<3; i++ )
    {
        output.Pos = input[i].Pos;
        TriStream.Append( output );
        
        uint iNext = (i+1)%3;
        output.Pos = input[iNext].Pos;
        TriStream.Append( output );
        
        output.Pos = centerPos;
        TriStream.Append( output );
        
        TriStream.RestartStrip();
    }
}
