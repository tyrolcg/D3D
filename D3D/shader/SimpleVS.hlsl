#include "SimpleShaderHeader.hlsli"

struct VSInput
{
	float3 Position : POSITION;
	float4 Color : COLOR;
};

struct VSOutput
{
	float4 Position : SV_POSITION; // SV(�V�X�e���l�FSystem Value)
	float4 Color : COLOR;
};
// register(b0) �͒萔�o�b�t�@�p���W�X�^��0�Ԗڂ��g�����Ƃ�錾
cbuffer Transform : register(b0)
{
	/*
	packoffset �͐擪�A�h���X����̃I�t�Z�b�g���w�肷��
	c1 �� float 4�� 16byte���\��
	c4 �� float 16���A64byte���\��
	c8 �� float 32���A4x32 = 128byte���\��
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