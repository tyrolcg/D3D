struct VSInput
{
	float3 Position : POSITION;
	float2 Uv : TEXCOORD;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
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

Texture2D tex : register(t0);
SamplerState samp : register(s0);

float4 ps_main(VSOutput input) : SV_TARGET
{
	return tex.Sample(samp, input.Uv);
}