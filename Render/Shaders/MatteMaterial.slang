import IMaterial;
import ISurfaceInteraction;
import Texture;

struct LambertianBSDF : IBSDF
{
    //Reflectance
    float3 R;

    float3 f(float3 wo, float3 wi)
    {
        return R / 3.141592654;
    }
};

struct MatteMaterialC : IMaterial
{
    typedef LambertianBSDF B;

	ConstantTexture Texture;
    
    B ComputeScatteringFunctions(ISurfaceInteraction isect)
    {
        LambertianBSDF result;
        result.R = Texture.Sample(isect.GetUV0()).rgb;
        return result;
    }
};

struct MatteMaterialI : IMaterial
{
    typedef LambertianBSDF B;

	ImageTexture Texture;
    
    B ComputeScatteringFunctions(ISurfaceInteraction isect)
    {
        LambertianBSDF result;
        result.R = Texture.Sample(isect.GetUV0()).rgb;
        return result;
    }
};
