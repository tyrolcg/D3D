////// Disney BRDF
////// https://media.disneyanimation.com/uploads/production/publication_asset/48/asset/s2012_pbs_disney_brdf_notes_v3.pdf
//// memoRandom : https://rayspace.xyz/CG/contents/Disney_principled_BRDF/
//struct VSInput
//{
//	float3 Position : POSITION;
//	float3 Normal   : NORMAL;
//	float2 Uv       : TEXCOORD0;
//	float3 Tangent  : TANGENT;
//};
//
//struct VSOutput
//{
//	float4 Position : SV_POSITION;
//	float3 Normal   : TEXCOORD2;
//	float2 Uv       : TEXCOORD0;
//	float3 WorldPos : TEXCOORD1;
//	float3 Tangent  : TEXCOORD3;
//};
//
//// 座標変換行列
//cbuffer Transform : register(b0)
//{
//	float4x4 World : packoffset(c0);
//	float4x4 View : packoffset(c4);
//	float4x4 Proj : packoffset(c8);
//}
//
//// ポイントライト情報
//cbuffer PointLightBuffer : register(b1)
//{
//	float3 LightPosition;
//	float  LightIntensity;
//	float3 LightColor;
//	float  LightAttenuation;
//}
//
//// PBRマテリアルパラメータ
//cbuffer MaterialParams : register(b2)
//{
//	float3 BaseColor;
//	float Metalic;
//	float Roughness;
//	float Subsurface;
//	float Specular; // スペキュラーの強さ
//	float SpecularTint; // スペキュラーの色をベースに近づける度合い
//	float Anisotropic; // ハイライトの異方性
//	float Sheen; // 布のような表面の微小なハイライト
//	float SheenTint; // sheenの色をベースに近づける度合い
//	float Clearcoat; // スペキュラーローブの強さ
//	float ClearcoatGloss; // スペキュラーローブの光沢度
//	float AmbientFactor; // 環境光係数
//}
//
//// カメラ位置
//cbuffer CameraBuffer : register(b3)
//{
//	float3 CameraPosition;
//	float3 CameraDirection;
//	float3 CameraUp;
//}
//// テクスチャとサンプラー
//// NOTE: 使わないけどCookTorranceでも定義してあるので合わせる
//Texture2D    AlbedoMap : register(t0);
//SamplerState DefaultSampler : register(s0);
//
//
//// 頂点シェーダー
//VSOutput main(VSInput input)
//{
//	VSOutput output = (VSOutput)0;
//
//	// 座標変換
//	float4 localPos = float4(input.Position, 1.0f);
//	float4 worldPos = mul(World, localPos);
//	float4 viewPos = mul(View, worldPos);
//	float4 projPos = mul(Proj, viewPos);
//
//	output.Position = projPos;
//	output.Normal = normalize(mul((float3x3)World, input.Normal));
//	output.Uv = input.Uv;
//	output.WorldPos = worldPos.xyz;
//	output.Tangent = normalize(mul((float3x3)World, input.Tangent));
//
//	return output;
//}
//
//// 定数
//float PI()
//{
//	return 3.14159265359;
//}
//
//// ヘルパー関数
//
//inline float SafeRcp(float x) { return (x > 1e-6) ? (1.0 / x) : 0.0; }
//inline float Sq(float x) { return x * x; }
//
//// roughness -> alpha (Disney 標準)
//inline float RoughnessToAlpha(float roughness)
//{
//	// 保持：ゼロ除算回避のため最小値を置く
//	roughness = max(roughness, 1e-4);
//	return roughness * roughness;
//}
//
////-------------------------
//// Disney roughness→αx,αy 変換
//// anisotropy ∈ [-1, 1]
////-------------------------
//// anisotropy = 0 → isotropic（αx = αy）
//// anisotropy > 0 → tangent方向に細長いハイライト
//// anisotropy < 0 → bitangent方向に細長いハイライト
//inline void ComputeAnisotropicAlpha(float roughness, float anisotropy, out float ax, out float ay)
//{
//	// Disney公式の異方性スケール変換
//	float aspect = sqrt(1.0 - 0.9 * anisotropy);
//	ax = max(1e-4, (roughness * roughness) / aspect);
//	ay = max(1e-4, (roughness * roughness) * aspect);
//}
///////////////////////////////////
//
//float3 F_t_Schlick(float NoV, float3 F90)
//{
//	return 1 + (F90 - 1) * pow(1 - NoV, 5);
//}
//
//float3 F_r_Schlick(float NoV, float3 F0)
//{
//	return F0 + (float3(1,1,1) - F0) * pow(1 - NoV, 5);
//}
//
//float F_D_90(float NoV, float roughness)
//{
//	return 0.5 + 2 * roughness * roughness * NoV * NoV;
//}
//
//float F_SS_90(float NoV, float roughness)
//{
//	return pow(NoV, 2) * roughness;
//}
//
//
//// 輝度(Y)の計算
//// NTSC色空間の輝度成分を使用
//// https://www.blog.urban-cafeteria.com/brightness-y-from-rgb/
//float Y_Brightness(float3 color)
//{
//	return 0.299 * color.r + 0.587 * color.g + 0.114 * color.b;
//}
//
//float3 R_tint(float3 baseColor)
//{
//	if (Y_Brightness(baseColor) == 0)
//	{
//		return 0;
//	}
//	return baseColor / Y_Brightness(baseColor);
//}
//
//float3 R_specular(float3 baseColor, float specularTint)
//{
//	return lerp(float3(1, 1, 1), R_tint(baseColor), specularTint);
//}
//
//
//float3 F_S0(float specular, float baseColor, float specularTint, float metalic)
//{
//	return lerp(0.08 * specular * R_specular(baseColor, specularTint), baseColor, metalic);
//}
//
//float3 R_sheen(float3 baseColor, float sheenTint)
//{
//	return lerp(1, R_tint(baseColor), sheenTint);
//}
//
//float3 F_Diffuse(float3 baseColor, float NoL, float NoV, float HoV, float roughness)
//{
//	float3 FL = F_t_Schlick(NoL, F_D_90(HoV, roughness));
//	float3 FV = F_t_Schlick(NoV, F_D_90(HoV, roughness));
//	float3 diffuse = baseColor / PI() * FL * FV;
//	return diffuse;
//}
//
//float3 F_Subsurface(float3 baseColor, float NoL, float NoV, float HoV, float roughness)
//{
//	float3 FL = F_t_Schlick(NoL, F_SS_90(HoV, roughness));
//	float3 FV = F_t_Schlick(NoV, F_SS_90(HoV, roughness));
//	float3 factor = 1 / (NoL + NoV + 0.001) - 0.5;
//	float3 subsurface = factor * FL * FV + 0.5;
//	subsurface *= baseColor * 1.25 / PI();
//	return subsurface;
//}
//
//float3 F_Sheen(float3 baseColor, float sheen, float sheenTint, float NoL, float NoV)
//{
//	float3 Rsheen = R_sheen(baseColor, sheenTint);
//	float3 FL = F_r_Schlick(NoL, Rsheen);
//	float3 FV = F_r_Schlick(NoV, Rsheen);
//	return sheen* R_sheen(baseColor, sheenTint)* FL* FV;
//}
//
//
//// 法線分布関数 GGX (Trowbridge-Reitz)
//float D_GGX_Aniso(float3 N, float3 H, float3 tangent, float3 bitangent, float ax, float ay)
//{
//	float3 Ht = float3(dot(H, tangent), dot(H, bitangent), dot(H, N));
//
//	float hx = Ht.x / ax;
//	float hy = Ht.y / ay;
//	float hz = Ht.z;
//
//	float denom = hx * hx + hy * hy + hz * hz;
//	float D = 1.0 / (PI() * ax * ay * denom * denom);
//	return D;
//}
//
//// 幾何減衰関数
//float G1_SmithGGX_Aniso(float3 N, float3 V, float3 tangent, float3 bitangent, float ax, float ay)
//{
//	float3 Vt = float3(dot(V, tangent), dot(V, bitangent), dot(V, N));
//
//	float vx = Vt.x * ax;
//	float vy = Vt.y * ay;
//	float vz = Vt.z;
//
//	float lambda = (-1.0 + sqrt(1.0 + (vx * vx + vy * vy) / (vz * vz))) * 0.5;
//	return 1.0 / (1.0 + lambda);
//}
//
//inline float G_SmithGGX_Aniso(float3 N, float3 V, float3 L, float3 tangent, float3 bitangent, float ax, float ay)
//{
//	float Gv = G1_SmithGGX_Aniso(N, V, tangent, bitangent, ax, ay);
//	float Gl = G1_SmithGGX_Aniso(N, L, tangent, bitangent, ax, ay);
//	return Gv * Gl;
//}
//
//// 擬似Schlick近似
//float G_SchlickSmith_Aniso_Approx(float3 N, float3 V, float3 L, float3 tangent, float3 bitangent, float ax, float ay)
//{
//	float NdotV = saturate(dot(N, V));
//	float NdotL = saturate(dot(N, L));
//
//	// 異方性粗さから「平均的な」roughnessを作る
//	float avgRough = sqrt(0.5 * (ax * ax + ay * ay));
//	float r = sqrt(avgRough);
//	float k = (r + 1.0);
//	k = (k * k) * 0.125; // (r+1)^2 / 8
//
//	float Gv = NdotV / (NdotV * (1.0 - k) + k);
//	float Gl = NdotL / (NdotL * (1.0 - k) + k);
//	return Gv * Gl;
//}
//
//float3 F_Specular(float3 baseColor, float specular, float specularTint, float metalic, float3 N, float3 H, float3 L, float3 V, float3 tangent, float3 bitangent, float ax, float ay)
//{
//	float NoV = max(saturate(dot(N, V)), 0.0001);
//	float NoL = max(saturate(dot(N, L)), 0.0001);
//
//	float3 F = F_r_Schlick(dot(N, V), F_S0(specular, Y_Brightness(baseColor), specularTint, metalic));
//
//	float D = D_GGX_Aniso(N, H, tangent, bitangent, ax, ay);
//
//	float G = G_SchlickSmith_Aniso_Approx(N, V, L, tangent, bitangent, ax, ay);
//
//	return (F * D * G) / (4.0 * NoL * NoV);
//}
//
//float3 F_Clearcoat(float3 F0, float clearcoat, float clearcoatGloss, float3 N, float3 H, float3 L, float3 V, float3 tangent, float3 bitangent, float ax, float ay)
//{
//	float NoV = max(saturate(dot(N, V)), 0.0001);
//	float NoL = max(saturate(dot(N, L)), 0.0001);
//	float roughness = lerp(0.1, 0.001, clearcoatGloss);
//	float alpha = RoughnessToAlpha(roughness);
//	// D
//	float D = D_GGX_Aniso(N, H, tangent, bitangent, ax, ay);
//	// G
//	float G = G_SchlickSmith_Aniso_Approx(N, V, L, tangent, bitangent, ax, ay);
//	// F
//	float3 F = F_r_Schlick(dot(N, V), F0);
//	return clearcoat * (F * D * G) / (4.0 * NoL * NoV);
//}
//
//// ピクセルシェーダー
//float4 ps_main(VSOutput input) : SV_TARGET
//{
//	// サンプルテクスチャ
//	//float4 albedo = AlbedoMap.Sample(DefaultSampler, input.Uv);
//	float4 albedo = float4(1.0, 1.0, 1.0, 1.0);
//	float3 baseColor = albedo.rgb * BaseColor;
//	// 基本的なベクトル
//	float3 N = normalize(input.Normal);
//	float3 V = normalize(CameraPosition - input.WorldPos); // カメラ位置が原点と仮定
//	float3 L = normalize(LightPosition - input.WorldPos);
//	float3 H = normalize(V + L);
//	// 内積
//	float NoL = max(dot(N, L), 0.001);
//	float NoV = max(dot(N, V), 0.001);
//	float NoH = max(dot(N, H), 0.001);
//	float HoV = max(dot(H, V), 0.001);
//	// 距離減衰
//	float dist = length(LightPosition - input.WorldPos);
//	float attenuation = 1.0 / (1.0 + LightAttenuation * dist * dist);
//	// メタリック/ラフネスワークフロー
//	float3 F0 = lerp(float3(0.04, 0.04, 0.04), baseColor.rgb, Metalic);
//	// 異方性α計算
//	float ax, ay;
//	ComputeAnisotropicAlpha(Roughness, Anisotropic, ax, ay);
//
//	// BRDF計算
//	float3 diffuseCol = F_Diffuse(baseColor, NoL, NoV, HoV, Roughness);
//	float3 subsurfaceCol = F_Subsurface(baseColor, NoL, NoV, HoV, Roughness);
//	float3 sheenCol = F_Sheen(baseColor, Sheen, SheenTint, NoL, NoV);
//	float3 specularCol = F_Specular(baseColor, Specular, SpecularTint, Metalic, N, H, L, V, normalize(input.Tangent), normalize(cross(N, input.Tangent)), ax, ay);
//	float3 clearcoatCol = F_Clearcoat(float3(0.04, 0.04, 0.04), Clearcoat, ClearcoatGloss, N, H, L, V, normalize(input.Tangent), normalize(cross(N, input.Tangent)), ax, ay);
//	float3 brdf = (lerp(diffuseCol, subsurfaceCol, Subsurface) + sheenCol) * (1.0 - Metalic) + specularCol + clearcoatCol;
//
//	// 最終的なライティング
//	float3 directLight = brdf * LightColor * LightIntensity * NoL * attenuation;
//
//	// 環境光
//	float3 ambient = baseColor * AmbientFactor;
//
//	// 最終的な色
//	float3 finalColor = directLight + ambient;
//
//	// ラインハルトトーンマッピング
//	finalColor = finalColor / (finalColor + 1.0);
//
//	finalColor = abs(finalColor);
//
//	// ガンマ補正
//	finalColor = pow(finalColor, 1.0 / 2.2);
//
//	return float4(finalColor, albedo.a);
//}