// Cook-Torrance BRDF シェーダー

struct VSInput
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 Uv       : TEXCOORD0;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float3 Normal   : TEXCOORD2;
    float2 Uv       : TEXCOORD0;
    float3 WorldPos : TEXCOORD1;
};

// 座標変換行列
cbuffer Transform : register(b0)
{
    matrix World;
    matrix View;
    matrix Proj;
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
    float  Metallic;       // 金属度（0=非金属、1=金属）
    float  Roughness;      // 粗さ（0=鏡面、1=ざらざら）
    float3 BaseColor;      // ベースカラー
    float  AmbientFactor;  // 環境光係数
}

// カメラ位置
cbuffer CameraBuffer : register(b3)
{
    float3 CameraPosition; // カメラ位置
    float3 CameraDirection; // カメラ方向
    float3 CameraUp; // カメラ上方向
}
// テクスチャとサンプラー
Texture2D    AlbedoMap     : register(t0);
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

    return output;
}

// 定数
static const float PI = 3.14159265359;
static const float3 F0_NON_METAL = float3(0.04, 0.04, 0.04);

// 法線分布関数 (GGX)
float D_GGX(float NoH, float roughness)
{
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float NoH2 = NoH * NoH;
    float denom = NoH2 * (alpha2 - 1.0) + 1.0;
    return alpha2 / (PI * denom * denom);
}

// 幾何減衰関数 (Smith GGX)
float G_Smith(float NoV, float NoL, float roughness)
{
    float alpha = roughness * roughness;
    float k = alpha / 2.0;
    float G1_V = NoV / (NoV * (1.0 - k) + k);
    float G1_L = NoL / (NoL * (1.0 - k) + k);
    return G1_V * G1_L;
}

// フレネル項 (Schlick近似)
float3 F_Schlick(float HoV, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - HoV, 5.0);
}

// ピクセルシェーダー
float4 ps_main(VSOutput input) : SV_TARGET
{
    // テクスチャ
    float4 albedo = AlbedoMap.Sample(DefaultSampler, input.Uv);
    float3 baseColor = albedo.rgb * BaseColor;

    // 基本的なベクトル
    float3 N = normalize(input.Normal);
    float3 V = normalize(CameraPosition - input.WorldPos);
    float3 L = normalize(LightPosition - input.WorldPos);
    float3 H = normalize(V + L);

    // 内積
    float NoL = max(dot(N, L), 0.001);
    float NoV = max(dot(N, V), 0.001);
    float NoH = max(dot(N, H), 0.001);
    float HoV = max(dot(H, V), 0.001);

    // 距離減衰
    float dist = length(LightPosition - input.WorldPos);
    float attenuation = 1.0 / (1.0 + LightAttenuation * dist * dist);

    // メタリック/ラフネスワークフロー
    float3 F0 = lerp(F0_NON_METAL, baseColor.rgb, Metallic);

    // マイクロファセットモデル
    float D = D_GGX(NoH, Roughness);
    float G = G_Smith(NoV, NoL, Roughness);
    float3 F = F_Schlick(HoV, F0);

    float3 specular = (D * G * F) / (4.0 * NoV * NoL + 0.001);

    // 拡散反射（金属の場合は拡散反射なし）
    float3 kD = (1.0 - F) * (1.0 - Metallic);
    float3 diffuse = kD * baseColor / PI;

    // 最終的なライティング
    float3 directLight = (diffuse + specular) * LightColor * LightIntensity * NoL * attenuation;

    // 環境光
    float3 ambient = baseColor * AmbientFactor;

    // 最終的な色
    float3 finalColor = directLight + ambient;

    // ラインハルトトーンマッピング
    finalColor = finalColor / (finalColor + 1.0);

    // ガンマ補正
    finalColor = pow(finalColor, 1.0/2.2);

    return float4(finalColor, albedo.a);
}