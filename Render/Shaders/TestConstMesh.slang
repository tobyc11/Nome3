//A vertex shader used for testing, no input buffers required

import ISurfaceInteraction;
import Camera;
import SceneObject;

struct TestConstMeshVSIn
{
    uint VertexId : SV_VertexId;
};

//Generate a vector perpendicular to u
float3 PerpHughesMoller(float3 u)
{
    float3 a = abs(u);
    float3 v;
    if (a.x <= a.y && a.x <= a.z)
        v = float3(0, -u.z, u.y);
    else if (a.y <= a.x && a.y <= a.z)
        v = float3(-u.z, 0, u.x);
    else
        v = float3(-u.y, u.x, 0);
    return v;
}

struct TestConstMeshVSOut : ISurfaceInteraction
{
	float4 PositionNDC : SV_POSITION;
	float3 WorldPosition : POSITION;
	float3 WorldNormal : NORMAL;
	float3 TangentWo : TANGENTWO;

    float3 GetWorldPosition()
	{
		return WorldPosition;
	}

    float3 GetWorldNormal()
	{
		return WorldNormal;
	}

    float3 GetTangentWo()
	{
		return TangentWo;
	}

    float3 WorldToTangent(float3 worldVec)
	{
		return mul(worldVec, GetTangentBasis());
	}

	float3x3 GetTangentBasis()
	{
		float3 i = PerpHughesMoller(WorldNormal);
		float3 j = WorldNormal;
		float3 k = cross(i, j);
		float3x3 T = float3x3(i, j, k);
		return T;
	}

    float2 GetUV0()
	{
		return float2(0, 0);
	}
};

TestConstMeshVSOut TestConstMeshVSMain(TestConstMeshVSIn input)
{
	TestConstMeshVSOut output;
	float4 pos;
	if (input.VertexId == 0)
        pos = float4(0.0, 0.5, 0.5, 1.0);
    else if (input.VertexId == 2)
        pos = float4(0.5, -0.5, 0.5, 1.0);
    else if (input.VertexId == 1)
        pos = float4(-0.5, -0.5, 0.5, 1.0);
	float4 worldPos = mul(pos, ModelMat);
	float4 eyePos = mul(worldPos, ViewMat);
	output.PositionNDC = mul(eyePos, ProjMat);
	output.WorldPosition = worldPos.xyz;
	output.WorldNormal = float3(0, 0, 1);
	output.TangentWo = output.WorldToTangent(CameraPos - output.WorldPosition);
	return output;
}
