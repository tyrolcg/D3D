//// Pixel shader (CookTorrancePS.hlsl)
//
//struct VSOutput
//{
//    float4 Position : SV_POSITION;
//    float3 Normal   : TEXCOORD2;
//    float2 Uv       : TEXCOORD0;
//    float3 WorldPos : TEXCOORD1;
//    float3 Tangent  : TEXCOORD3;
//};
//
//// ポイントライト情報
//cbuffer PointLightBuffer : register(b1)
//{
//    float3 LightPosition;
//    float  LightIntensity;
//    float3 LightColor;
//    float  LightAttenuation;
//}
//
//// PBRマテリアルパラメータ
//cbuffer MaterialParams : register(b2)
//{
//    	float3 BaseColor;
//    	float Metallic;
//    	float Roughness;
//    	float Subsurface;
//    	float Specular; // スペキュラーの強さ
//    	float SpecularTint; // スペキュラーの色をベースに近づける度合い
//    	float Anisotropic; // ハイライトの異方性
//    	float Sheen; // 布のような表面の微小なハイライト
//    	float SheenTint; // sheenの色をベースに近づける度合い
//    	float Clearcoat; // スペキュラーローブの強さ
//    	float ClearcoatGloss; // スペキュラーローブの光沢度
//    	float AmbientFactor; // 環境光係数
//}
//
//// カメラ位置
//cbuffer CameraBuffer : register(b3)
//{
//    float3 CameraPosition;
//    float3 CameraDirection;
//    float3 CameraUp;
//}
//
//// テクスチャとサンプラー
//Texture2D    AlbedoMap     : register(t0);
//SamplerState DefaultSampler : register(s0);
//
//// 定数
//static const float PI = 3.14159265359;
//static const float3 F0_NON_METAL = float3(0.04, 0.04, 0.04);
//
//// 法線分布関数 (GGX)
//float D_GGX(float NoH, float roughness)
//{
//    float alpha = roughness * roughness;
//    float alpha2 = alpha * alpha;
//    float NoH2 = NoH * NoH;
//    float denom = NoH2 * (alpha2 - 1.0) + 1.0;
//    return alpha2 / (PI * denom * denom);
//}
//
//// 幾何減衰関数 (Smith GGX)
//float G_Smith(float NoV, float NoL, float roughness)
//{
//    float alpha = roughness * roughness;
//    float k = alpha / 2.0;
//    float G1_V = NoV / (NoV * (1.0 - k) + k);
//    float G1_L = NoL / (NoL * (1.0 - k) + k);
//    return G1_V * G1_L;
//}
//
//// フレネル項 (Schlick近似)
//float3 F_Schlick(float HoV, float3 F0)
//{
//    return F0 + (1.0 - F0) * pow(1.0 - HoV, 5.0);
//}
//
//float4 ps_main(VSOutput input) : SV_TARGET
//{
//    float4 albedo = float4(1.0, 1.0, 1.0, 1.0);
//    float3 baseColor = albedo.rgb * BaseColor;
//
//    float3 N = normalize(input.Normal);
//    float3 V = normalize(CameraPosition - input.WorldPos);
//    float3 L = normalize(LightPosition - input.WorldPos);
//    float3 H = normalize(V + L);
//
//    float NoL = max(dot(N, L), 0.001);
//    float NoV = max(dot(N, V), 0.001);
//    float NoH = max(dot(N, H), 0.001);
//    float HoV = max(dot(H, V), 0.001);
//
//    float dist = length(LightPosition - input.WorldPos);
//    float attenuation = 1.0 / (1.0 + LightAttenuation * dist * dist);
//
//    float3 F0 = lerp(F0_NON_METAL, baseColor.rgb, Metallic);
//
//    float D = D_GGX(NoH, Roughness);
//    float G = G_Smith(NoV, NoL, Roughness);
//    float3 F = F_Schlick(HoV, F0);
//
//    float3 specular = (D * G * F) / (4.0 * NoV * NoL + 0.001);
//    float3 kD = (1.0 - F) * (1.0 - Metallic);
//    float3 diffuse = kD * baseColor / PI;
//
//    float3 directLight = (diffuse + specular) * LightColor * LightIntensity * NoL * attenuation;
//    float3 ambient = baseColor * AmbientFactor;
//    float3 finalColor = directLight + ambient;
//    finalColor = finalColor / (finalColor + 1.0);
//    finalColor = pow(finalColor, 1.0/2.2);
//
//    return float4(finalColor, albedo.a);
//}