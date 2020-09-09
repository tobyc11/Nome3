import ISurfaceInteraction;

interface IBSDF
{
    float3 f(float3 wo, float3 wi);
};

interface IMaterial
{
    associatedtype B : IBSDF;

    B ComputeScatteringFunctions(ISurfaceInteraction isect);
};
