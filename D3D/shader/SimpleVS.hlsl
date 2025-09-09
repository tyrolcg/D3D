#include "SimpleShaderHeader.hlsli"

struct VSInput
{
	float3 Position : POSITION;
	float4 Color : COLOR;
};

struct VSOutput
{
	float4 Position : SV_POSITION; // SV(システム値：System Value)
	float4 Color : COLOR;
};
// register(b0) は定数バッファ用レジスタの0番目を使うことを宣言
cbuffer Transform : register(b0)
{
	/*
	packoffset は先頭アドレスからのオフセットを指定する
	c1 は float 4つ分 16byte先を表す
	c4 は float 16個分、64byte先を表す
	c8 は float 32個分、4x32 = 128byte先を表す
	*/
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
	output.Color = input.Color;

	return output;
}