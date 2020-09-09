import ISurfaceInteraction;
import IMaterial;

//There seems no other way to include those conditional dependencies
import TestConstMesh;
import StaticMesh;
import MatteMaterial;

type_param TSurfaceInteraction : ISurfaceInteraction;

type_param TMaterial : IMaterial;

struct DirectLightingPSOut
{
    float4 Target0 : SV_Target0;
}

ParameterBlock<TMaterial> material;

float3 Accum(TSurfaceInteraction isect, float3 lightPos, float3 intensity)
{
    float3 N = normalize(isect.GetWorldNormal());
    float dist = length(lightPos - isect.GetWorldPosition());
    float fallOff = 1 / dist / dist;
    float3 L = normalize(lightPos - isect.GetWorldPosition());
    float NoL = dot(N, L);
    return intensity * fallOff * NoL;
}

DirectLightingPSOut DirectLightingPSMain(TSurfaceInteraction isect)
{
    DirectLightingPSOut output;
    IBSDF bsdf = material.ComputeScatteringFunctions(isect);
    float3 f = bsdf.f(isect.GetTangentWo(), float3(1));

    float3 Lo = float3(0);

    Lo += Accum(isect, float3(0, 1, -0.2), float3(0.5));
    Lo += Accum(isect, float3(2, 1, -0.2), float3(0.5));
    Lo += Accum(isect, float3(4, 1, -0.2), float3(0.5));
    Lo += Accum(isect, float3(6, 1, 0), float3(0.5));
    Lo += Accum(isect, float3(-8, 1, 0), float3(0.5));
    Lo += f * float3(0.7) * dot(normalize(isect.GetWorldNormal()), float3(1));

    output.Target0 = float4(Lo, 1);
    //output.Target0 = float4(f * dot(normalize(isect.GetWorldNormal()), float3(1)), 1); //BSDF visualization
    return output;
}
