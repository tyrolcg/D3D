struct VSInput
{
	float3 Position : POSITION;
	float2 Uv : TEXCOORD;
};

struct VSOutput
{
	float4 Position : SV_POSITION;
	float2 Uv : TEXCOORD;
};

cbuffer Transform : register(b0)
{
	float4x4 World : packoffset(c0);
	float4x4 View : packoffset(c4);
	float4x4 Proj : packoffset(c8);
}

VSOutput main(VSInput input) 
{
	VSOutput output = (VSOutput)0;
	float4 localPos = float4(input.Position, 1.0f);
	float4 worldPos = mul(World, localPos);
	float4 viewPos = mul(View, worldPos);
	float4 projPos = mul(Proj, viewPos);
	output.Position = projPos;

	output.Uv = input.Uv;

	return output;
}

struct PSOutput
{
	float4 Color : SV_TARGET0;
};

Texture2D tex : register(t0);
SamplerState samp : register(s0);

PSOutput main(VSOutput input)
{
	PSOutput output = (PSOutput)0;

	output.Color = tex.Sample(samp, input.Uv);

	return output;
}