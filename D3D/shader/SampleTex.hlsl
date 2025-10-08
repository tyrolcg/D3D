struct VSInput
{
	float3 Position : POSITION;
	float3 Normal   : NORMAL;
	float2 Uv       : TEXCOORD;
};

struct VSOutput
{
	float4 Position : SV_POSITION;
	float3 Normal   : TEXCOORD2;
	float2 Uv : TEXCOORD;
	float3 WorldPos : TEXCOORD1;
};

cbuffer Transform : register(b0)
{
	float4x4 World : packoffset(c0);
	float4x4 View : packoffset(c4);
	float4x4 Proj : packoffset(c8);
}

cbuffer PointLightBuffer : register(b1)
{
	float3 LightPosition;
	float  LightIntensity;
	float3 LightColor;
	float  LightAttenuation;
}

VSOutput main(VSInput input) 
{
	VSOutput output = (VSOutput)0;
	float4 localPos = float4(input.Position, 1.0f);
	float4 worldPos = mul(World, localPos);
	float4 viewPos = mul(View, worldPos);
	float4 projPos = mul(Proj, viewPos);
	output.Position = projPos;

	output.Normal = mul((float3x3)World, input.Normal); // ワールド空間に変換
	output.Normal = normalize(output.Normal);

	output.Uv = input.Uv;

	output.WorldPos = worldPos.xyz;

	return output;
}

Texture2D tex : register(t0);
SamplerState samp : register(s0);

float4 ps_main(VSOutput input) : SV_TARGET
{
    float3 normal = input.Normal;
	float3 fragPos = input.WorldPos;
    float3 lightDir = LightPosition - fragPos;
    float dist = length(lightDir);
    lightDir = normalize(lightDir);
    float attenuation = 1.0f / (1.0f + LightAttenuation * dist * dist);
    float NdotL = max(dot(normal, lightDir), 0.0f);
    float3 diffuse = LightColor * LightIntensity * NdotL * attenuation;
    float4 texColor = tex.Sample(samp, input.Uv);
    return float4(texColor.rgb * diffuse, 1.0f);
}