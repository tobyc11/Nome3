interface ITexture
{
	float4 Sample(float2 uv);
};

struct ConstantTexture : ITexture
{
	float4 Constant;

	float4 Sample(float2 uv)
	{
		return Constant;
	}
};

struct ImageTexture : ITexture
{
    Texture2D Image;
    SamplerState Sampler;
	
	float4 Sample(float2 uv)
	{
		return Image.Sample(Sampler, uv);
	}
};
