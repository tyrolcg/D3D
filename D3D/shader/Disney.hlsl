//// Disney BRDF
//// https://media.disneyanimation.com/uploads/production/publication_asset/48/asset/s2012_pbs_disney_brdf_notes_v3.pdf
// memoRandom : https://rayspace.xyz/CG/contents/Disney_principled_BRDF/
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

// ポイントライト情報
cbuffer PointLightBuffer : register(b1)
{
	float3 LightPosition;
	float  LightIntensity;
	float3 LightColor;
	float  LightAttenuation;
}

// PBRマテリアルパラメータ
cbuffer MaterialParams : register(b2)
{
	float3 BaseColor;
	float Metalic;
	float Roughness;
	float specular; // スペキュラーの強さ
	float specularTint; // スペキュラーの色をベースに近づける度合い
	float anisotropic; // ハイライトの異方性
	float sheen; // 布のような表面の微小なハイライト
	float sheenTint; // sheenの色をベースに近づける度合い
	float clearcoat; // スペキュラーローブの強さ
	float clearcoatGloss; // スペキュラーローブの光沢度
}

// カメラ位置
cbuffer CameraBuffer : register(b3)
{
	float3 CameraPosition;
	float3 CameraDirection;
	float3 CameraUp;
}
// テクスチャとサンプラー
// NOTE: 使わないけどCookTorranceでも定義してあるので合わせる
Texture2D    AlbedoMap : register(t0);
SamplerState DefaultSampler : register(s0);


// 頂点シェーダー
VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput)0;

	// 座標変換
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

// 定数
float PI = 3.14159265359;

float3 F_t_Schlick(float NoV, float3 F90)
{
	return 1 + (F90 - 1) * pow(1 - NoV, 5);
}

float3 F_r_Schlick(float NoV, float3 F0)
{
	return F0 + (1 - F0) * pow(1 - NoV, 5);
}

float F_D_90(float NoV, float roughness)
{
	return 0.5 + 2 * roughness * roughness * NoV * NoV;
}

float F_SS_90(float NoV, float roughness)
{
	return pow(NoV, 2) * roughness;
}

float R_tint(float3 baseColor)
{
	if (Y_Brightness(baseColor) == 0)
	{
		return 0;
	}
	return baseColor / Y_Brightness(baseColor);
}

// 輝度(Y)の計算
// NTSC色空間の輝度成分を使用
// https://www.blog.urban-cafeteria.com/brightness-y-from-rgb/
float Y_Brightness(float3 color)
{
	return 0.299 * color.r + 0.587 * color.g + 0.114 * color.b;
}

float F_S0(float specular, float baseColor, float specularTint, float metalic)
{
	return lerp(0.08 * specular * R_specular(baseColor, specularTint), baseColor, metalic);
}

float R_sheen(float3 baseColor, float sheenTint)
{
	return lerp(1, R_tint(baseColor), sheenTint);
}

float R_specular(float3 baseColor, float specularTint)
{
	return lerp(float3(1, 1, 1), R_tint(baseColor), specularTint);
}

float F_Diffuse(float3 baseColor, float NoL, float NoV, float HoV, float roughness)
{
	float FL = F_t_Schlick(NoL, F_D_90(HoV, roughness));
	float FV = F_t_Schlick(NoV, F_D_90(HoV, roughness));
	float3 diffuse = baseColor / PI * FL * FV;
	return diffuse;
}

float F_Subsurface(float3 baseColor, float NoL, float NoV, float HoV, float roughness)
{
	float FL = F_t_Schlick(NoL, F_SS_90(HoV, roughness));
	float FV = F_t_Schlick(NoV, F_SS_90(HoV, roughness));
	float factor = 1 / (NoL + NoV + 0.001) - 0.5;
	float subsurface = factor * FL * FV + 0.5;
	subsurface *= baseColor * 1.25 / PI;
	return subsurface;
}

float F_sheen(float3 baseColor, float sheen, float sheenTint, float NoL, float NoV)
{
	float3 Rsheen = R_sheen(baseColor, sheenTint);
	float FL = F_r_Schlick(NoL, Rsheen);
	float FV = F_r_Schlick(NoV, Rsheen);
	sheen* R_sheen(baseColor, sheenTint)* FL* FV;
}