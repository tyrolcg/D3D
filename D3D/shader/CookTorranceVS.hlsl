// Vertex shader (CookTranceVS.hlsl)

struct VSInput
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 Uv       : TEXCOORD0;
    float3 Tangent  : TANGENT;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float3 Normal   : TEXCOORD2;
    float2 Uv       : TEXCOORD0;
    float3 WorldPos : TEXCOORD1;
    float3 Tangent  : TEXCOORD3;
};

// 座標変換行列
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
    output.Normal = normalize(mul((float3x3)World, input.Normal));
    output.Uv = input.Uv;
    output.WorldPos = worldPos.xyz;
    output.Tangent = normalize(mul((float3x3)World, input.Tangent));

    return output;
}