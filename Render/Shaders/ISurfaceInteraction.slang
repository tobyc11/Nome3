interface ISurfaceInteraction
{
    float3 GetWorldPosition();
    float3 GetWorldNormal();
    float3 GetTangentWo();
    float3 WorldToTangent(float3 worldVec);
	float3x3 GetTangentBasis();
    float2 GetUV0();
};
