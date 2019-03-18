import ISurfaceInteraction;
import Camera;
import SceneObject;

struct StaticMeshVSIn
{
    float3 Position : ATTRIBUTE0;
    float3 Normal : ATTRIBUTE1;
#ifdef HAS_TANGENT
	float4 Tangent : ATTRIBUTE2;
#endif
#ifdef HAS_UV0
    float2 UV0 : ATTRIBUTE3;
#endif
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

struct StaticMeshVSOut : ISurfaceInteraction
{
	float4 PositionNDC : SV_POSITION;
	float3 WorldPosition : POSITION;
	float3 WorldNormal : NORMAL;
#ifdef HAS_UV0
	float2 UV0 : TEXCOORD0;
#endif
	float3 TangentWo : TANGENTWO;
#ifdef HAS_TANGENT
	float3 Tangent : TANGENT;
	float3 BiTangent : BITANGENT;
#endif

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
#ifdef HAS_TANGENT
		float3x3 T = float3x3(Tangent, WorldNormal, BiTangent);
#else
		float3 i = PerpHughesMoller(WorldNormal);
		float3 j = WorldNormal;
		float3 k = cross(i, j);
		float3x3 T = float3x3(i, j, k);
#endif
		return T;
	}

    float2 GetUV0()
	{
#ifdef HAS_UV0
		return UV0;
#else
		return float2(0);
#endif
	}
};

StaticMeshVSOut StaticMeshVSMain(StaticMeshVSIn input)
{
	StaticMeshVSOut output;
	float4 pos = float4(input.Position, 1);
	float4 worldPos = mul(pos, ModelMat);
	float4 eyePos = mul(worldPos, ViewMat);
	output.PositionNDC = mul(eyePos, ProjMat);
	output.WorldPosition = worldPos.xyz;
	output.WorldNormal = mul(float4(input.Normal, 1), NormalMat).xyz;
#ifdef HAS_UV0
	output.UV0 = input.UV0;
#endif
#ifdef HAS_TANGENT
	output.Tangent = input.Tangent.xyz;
	output.BiTangent = cross(input.Normal, input.Tangent.xyz) * input.Tangent.w;
#endif
	output.TangentWo = output.WorldToTangent(CameraPos - output.WorldPosition);

	return output;
}
